#!/usr/bin/env python
#
# Test python application to send test message to server
#
# Shows connection detail so we can compare it to Sming code
#

import asyncio
from websockets.sync.client import connect
import logging

def hello():
    logging.basicConfig(
        format="%(message)s",
        level=logging.DEBUG,
    )
    with connect("ws://localhost:8000") as websocket:
        websocket.send("Hello world!")
        message = websocket.recv()
        print(f"Received: {message}")

if __name__ == "__main__":
    hello()
