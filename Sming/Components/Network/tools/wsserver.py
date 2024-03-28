#!/usr/bin/env python

import asyncio
from websockets.server import serve
import logging

async def echo(websocket):
    async for message in websocket:
        await websocket.send(message)

async def main():
    logging.basicConfig(
        format="%(asctime)s %(message)s",
        level=logging.DEBUG,
    )
    async with serve(echo, None, 8000):
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    asyncio.run(main())
