import zmq
from zmq.asyncio import Context, Socket
import asyncio
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ZMQPublisher:
    def __init__(self, endpoint="tcp://127.0.0.1:5555"):
        self.endpoint = endpoint
        self.context = Context.instance()
        self.socket: Socket = self.context.socket(zmq.PUB)

        try:
            self.socket.bind(self.endpoint)
            logger.info(f"[ZMQPublisher] Bound to {self.endpoint}")
        except zmq.ZMQError as e:
            logger.error(f"[ZMQPublisher] Failed to bind to {self.endpoint}: {e}")
            raise

        # Socket options
        self.socket.setsockopt(zmq.SNDHWM, 100000)  # high-water mark
        self.socket.setsockopt(zmq.LINGER, 0)       # drop unsent on close

    async def publish(self, topic: str, fb_bytes: bytes):
        """Send FlatBuffers bytes over ZeroMQ with topic prefix."""
        try:
            await self.socket.send_multipart([topic.encode(), fb_bytes])
            await asyncio.sleep(0)  # yield to event loop to prevent lockups
        except zmq.ZMQError as e:
            logger.error(f"[ZMQPublisher] Send error: {e}")

    async def close(self):
        """Cleanly close socket."""
        if not self.socket.closed:
            self.socket.close(linger=0)
            logger.info(f"[ZMQPublisher] Closed socket at {self.endpoint}")