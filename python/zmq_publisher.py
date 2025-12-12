import zmq
from zmq.asyncio import Context, Socket
import asyncio
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ZMQPublisher:
    def __init__(self, endpoint="tcp://127.0.0.1:5555", max_retries=5, retry_delay=0.5):
        """
        :param endpoint: ZMQ PUB endpoint to bind to
        :param max_retries: number of times to retry binding if address is in use
        :param retry_delay: delay (seconds) between retries
        """
        self.endpoint = endpoint
        self.context = Context.instance()
        self.socket: Socket = self.context.socket(zmq.PUB)

        # Socket options
        self.socket.setsockopt(zmq.SNDHWM, 100_000)
        self.socket.setsockopt(zmq.LINGER, 0)

        # Attempt binding
        for attempt in range(1, max_retries + 1):
            try:
                self.socket.bind(self.endpoint)
                logger.info(f"[ZMQPublisher] Bound to {self.endpoint} on attempt {attempt}")
                break
            except zmq.ZMQError as e:
                logger.warning(f"[ZMQPublisher] Attempt {attempt} failed to bind {self.endpoint}: {e}")
                if attempt == max_retries:
                    logger.error(f"[ZMQPublisher] Failed to bind after {max_retries} attempts.")
                    raise
                asyncio.sleep(retry_delay)

    async def publish(self, topic: str, fb_bytes: bytes):
        """Send FlatBuffers bytes over ZeroMQ with topic prefix."""
        try:
            await self.socket.send_multipart([topic.encode(), fb_bytes])
            logger.debug(f"[ZMQPublisher] Published message on topic: {topic} ({len(fb_bytes)} bytes)")
            await asyncio.sleep(0)  # yield to event loop
        except zmq.ZMQError as e:
            logger.error(f"[ZMQPublisher] Send error: {e}")

    async def close(self):
        """Cleanly close socket."""
        if not self.socket.closed:
            self.socket.close(linger=0)
            logger.info(f"[ZMQPublisher] Closed socket at {self.endpoint}")
