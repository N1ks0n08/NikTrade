import asyncio
import json
import logging
from pathlib import Path
import signal

import aiohttp
import zmq
import zmq.asyncio

from crypto_connection import book_ticker_stream
from crypto_historical_data import fetch_historical_klines
from flatbuffer_encoder import encode_bookticker, encode_klines
from zmq_publisher import ZMQPublisher

# ------------------- Cross-platform ZMQ fix for Windows -------------------
import sys
if sys.platform.startswith("win"):
    if hasattr(asyncio, "WindowsSelectorEventLoopPolicy"):
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

# ------------------- Logging setup -------------------
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

SYMBOL_FILE = Path.cwd() / "binance_symbols.json"

# ------------------- Shutdown Event -------------------
shutdown_event = asyncio.Event()


async def fetch_binance_symbols():
    if SYMBOL_FILE.exists():
        logger.info(f"[INFO] Loading cached symbols from {SYMBOL_FILE}")
        with open(SYMBOL_FILE, "r") as f:
            return json.load(f)

    logger.info("[INFO] Fetching active symbols from Binance REST API...")
    url = "https://api.binance.us/api/v3/exchangeInfo"

    async with aiohttp.ClientSession() as session:
        async with session.get(url) as resp:
            resp.raise_for_status()
            data = await resp.json()

    symbols = [s["symbol"].lower() for s in data["symbols"] if s["status"] == "TRADING"]

    with open(SYMBOL_FILE, "w") as f:
        json.dump(symbols, f, indent=2)

    logger.info(f"[INFO] Saved {len(symbols)} trading pairs to {SYMBOL_FILE}")
    return symbols


async def stream_and_publish(symbols):
    publisher = ZMQPublisher("tcp://127.0.0.1:5555")
    publisher.socket.linger = 0  # release port immediately

    async def consume(sym):
        try:
            async for payload in book_ticker_stream(sym):
                if shutdown_event.is_set():
                    break
                try:
                    fb_bytes = encode_bookticker(payload)
                    await publisher.publish(f"bookticker.{sym}", fb_bytes)
                    await asyncio.sleep(0.05)
                except Exception as e:
                    logger.error(f"[ERROR] {sym}: {e}")
                    await asyncio.sleep(1)
        except asyncio.CancelledError:
            pass

    tasks = [asyncio.create_task(consume(sym)) for sym in symbols[:5]]

    try:
        await shutdown_event.wait()
    finally:
        for t in tasks:
            t.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)
        await publisher.close()
        logger.info("[INFO] BookTicker publisher closed.")


async def fetch_and_publish_klines(symbol: str):
    publisher = ZMQPublisher("tcp://127.0.0.1:5556")
    publisher.socket.linger = 0

    async with aiohttp.ClientSession() as session:
        candles = []
        async for candle in fetch_historical_klines(session, symbol, interval="1m", limit=400):
            if shutdown_event.is_set():
                break
            candles.append(candle)

        if candles:
            fb_bytes = encode_klines(candles)
            await publisher.publish(f"klines.{symbol}", fb_bytes)
            logger.info(f"[INFO] Published {len(candles)} historical candles for {symbol}")

    await publisher.close()
    logger.info("[INFO] Klines publisher closed.")


async def control_server():
    ctx = zmq.asyncio.Context()
    socket = ctx.socket(zmq.REP)
    socket.linger = 0
    socket.bind("tcp://127.0.0.1:5560")

    logger.info("[INFO] Control socket listening on tcp://127.0.0.1:5560")

    try:
        while not shutdown_event.is_set():
            try:
                msg = await asyncio.wait_for(socket.recv_string(), timeout=1)
                logger.info(f"[INFO] Control message received: {msg}")

                if msg.startswith("fire_klines"):
                    parts = msg.strip().split()
                    if len(parts) == 2:
                        symbol = parts[1].lower()
                        await fetch_and_publish_klines(symbol)
                        await socket.send_string(f"OK: fired klines for {symbol}")
                    else:
                        await socket.send_string("ERROR: invalid command format")
                else:
                    await socket.send_string("ERROR: unknown command")
            except asyncio.TimeoutError:
                continue
    except asyncio.CancelledError:
        pass
    finally:
        await socket.close()
        logger.info("[INFO] Control server socket closed.")


async def main():
    symbols = await fetch_binance_symbols()
    tasks = [
        asyncio.create_task(stream_and_publish(symbols)),
        asyncio.create_task(fetch_and_publish_klines(symbols[0])),
        asyncio.create_task(control_server()),
    ]
    await shutdown_event.wait()
    for t in tasks:
        t.cancel()
    await asyncio.gather(*tasks, return_exceptions=True)


def shutdown_handler():
    logger.info("[INFO] Shutdown signal received. Cleaning up...")
    shutdown_event.set()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("[INFO] Ctrl+C detected. Shutting down...")
        shutdown_event.set()

