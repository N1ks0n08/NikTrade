import asyncio
import json
import logging
from pathlib import Path
import time

import aiohttp
import zmq
import zmq.asyncio

from crypto_connection import book_ticker_stream
from crypto_historical_data import fetch_historical_klines
from flatbuffer_encoder import encode_bookticker, encode_klines
from zmq_publisher import ZMQPublisher

import sys

if sys.platform.startswith("win"):
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

# ------------------- Logging setup -------------------
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

SYMBOL_FILE = Path.cwd() / "binance_symbols.json"

# ------------------- Shutdown Event -------------------
shutdown_event = asyncio.Event()

# ------------------- Helper: Load Symbols -------------------
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

# ---------------------- Stream for a single symbol ----------------------
async def stream_for_symbol(symbol: str, publisher, latency_publisher):
    logger.info(f"[INFO] Starting stream task for {symbol}")
    last_msg_time = None

    try:
        async for payload in book_ticker_stream(symbol):
            if shutdown_event.is_set():
                break

            now = int(time.time() * 1000)
            latency = f"{now - last_msg_time} ms" if last_msg_time else "Loading..."
            last_msg_time = now

            # Publish latency
            await latency_publisher.publish("feed_latency", f"{symbol} {latency}".encode())
            fb_bytes = encode_bookticker(payload)
            await publisher.publish(f"bookticker.{symbol}", fb_bytes)

            await asyncio.sleep(0.05)

    except asyncio.CancelledError:
        logger.info(f"[INFO] Stream task for {symbol} cancelled cleanly")
    except Exception as e:
        logger.exception(f"[ERROR] Unexpected error in stream_for_symbol({symbol}): {e}")

# ------------------- Historical Klines Task -------------------
async def fetch_and_publish_klines(symbol: str, klines_publisher: ZMQPublisher):
    try:
        async with aiohttp.ClientSession() as session:
            candles = []
            async for candle in fetch_historical_klines(session, symbol, interval="1m", limit=400):
                if shutdown_event.is_set():
                    break
                candles.append(candle)

            if candles:
                fb_bytes = encode_klines(candles)
                await klines_publisher.publish(f"klines.{symbol}", fb_bytes)
                logger.info(f"[INFO] Published {len(candles)} historical candles for {symbol}")

    except asyncio.CancelledError:
        pass
    except Exception as e:
        logger.error(f"[ERROR] Klines task {symbol}: {e}")

# ---------------------- REQ/REP Control Server ----------------------
async def control_server(stream_tasks: list, publisher, latency_publisher, klines_publisher, rep_endpoint="tcp://127.0.0.1:5560"):
    """
    REQ/REP server: handles start_symbol & fire_klines without blocking responses.
    Port: 5560
    """
    logger.info(f"[INFO] Control server listening at {rep_endpoint}")
    context = zmq.asyncio.Context.instance()
    socket = context.socket(zmq.REP)
    socket.bind(rep_endpoint)

    while not shutdown_event.is_set():
        try:
            msg = await socket.recv_string()
            logger.info(f"[INFO] Control message received: {msg}")
            parts = msg.strip().split()
            if len(parts) != 2:
                await socket.send_string("ERROR: invalid format")
                continue

            cmd, symbol = parts
            symbol = symbol.lower()

            if cmd == "start_symbol":
                try:
                    logger.info(f"[INFO] Starting stream to symbol: {symbol}")

                    # Cancel old streams
                    old_tasks = stream_tasks.copy()
                    for t in old_tasks:
                        t.cancel()
                    stream_tasks.clear()

                    # Start new stream
                    new_task = asyncio.create_task(stream_for_symbol(symbol, publisher, latency_publisher))
                    stream_tasks.append(new_task)
                    logger.info(f"[INFO] Stream task for {symbol} started")

                    # Respond immediately
                    await socket.send_string("OK")

                    # Cleanup old tasks in background correctly
                    async def cleanup(tasks):
                        await asyncio.gather(*tasks, return_exceptions=True)

                    asyncio.create_task(cleanup(old_tasks))
                    logger.info(f"[INFO] switch_symbol for {symbol} completed!!")

                except Exception as e:
                    logger.exception(f"[ERROR] switch_symbol failed for {symbol}: {e}")
                    # Only attempt to send error if socket is still usable
                    try:
                        await socket.send_string(f"ERROR: {e}")
                    except zmq.error.ZMQError:
                        logger.warning("ZMQ socket cannot send error message, skipping")


            elif cmd == "fire_klines":
                try:
                    logger.info(f"[INFO] Fetching historical klines for {symbol}")
                    # Run the fetch/publish task but wait here since it's usually quick
                    await fetch_and_publish_klines(symbol, klines_publisher)
                    await socket.send_string("OK")
                except Exception as e:
                    logger.error(f"[ERROR] Failed klines task for {symbol}: {e}")
                    await socket.send_string(f"ERROR: {e}")

            else:
                logger.warning(f"[WARN] Unknown control command: {cmd}")
                await socket.send_string(f"ERROR: unknown command {cmd}")

        except asyncio.CancelledError:
            logger.info("[INFO] Control server cancelled")
            break
        except Exception as e:
            logger.exception(f"[ERROR] Exception in control_server: {e}")
            await asyncio.sleep(0.1)

    socket.close()
    logger.info("[INFO] Control server exiting")


# ------------------- Main -------------------
async def main():
    symbols = await fetch_binance_symbols()

    publisher = ZMQPublisher("tcp://127.0.0.1:5555")
    latency_publisher = ZMQPublisher("tcp://127.0.0.1:5561")
    klines_publisher = ZMQPublisher("tcp://127.0.0.1:5556")

    stream_tasks = []

    # Start initial streams
    for sym in symbols[:2]:
        task = asyncio.create_task(stream_for_symbol(sym, publisher, latency_publisher))
        stream_tasks.append(task)

    # Start control server
    control_task = asyncio.create_task(control_server(stream_tasks, publisher, latency_publisher, klines_publisher))

    # Fetch historical klines for first symbol
    klines_task = asyncio.create_task(fetch_and_publish_klines(symbols[0], klines_publisher))

    await shutdown_event.wait()

    # Cleanup
    for t in stream_tasks:
        t.cancel()
    klines_task.cancel()
    control_task.cancel()
    await asyncio.gather(*stream_tasks, klines_task, control_task, return_exceptions=True)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        shutdown_event.set()
