import zmq
from zmq.asyncio import Context, Socket

context = Context.instance()

class ZMQPublisher:
    def __init__(self, endpoint="tcp://127.0.0.1:5555"):
        self.endpoint = endpoint
        self.socket: Socket = context.socket(zmq.PUB)
        self.socket.bind(self.endpoint)
        self.socket.setsockopt(zmq.SNDHWM, 100000)  # increase high-water mark
        self.socket.setsockopt(zmq.LINGER, 0)        # drop unsent messages on close
        print(f"ZMQ Publisher bound to {self.endpoint}")

    async def publish(self, topic: str, fb_bytes: bytes):
        """
        Send FlatBuffers bytes over ZMQ with a topic.
        """
        # ZMQ expects bytes for topic and message
        await self.socket.send_multipart([topic.encode(), fb_bytes])
    async def close(self):
        self.socket.close()
