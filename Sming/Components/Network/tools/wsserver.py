#!/usr/bin/env python

import argparse
import asyncio
from websockets.server import serve
import logging

async def echo(websocket):
    async for message in websocket:
        await websocket.send(message)

async def main(port: int):
    logging.basicConfig(
        format="%(asctime)s %(message)s",
        level=logging.DEBUG,
    )
    async with serve(ws_handler=echo, port=port):
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Simple websocket server')
    parser.add_argument('port', help='Port number (default 8000)', type=int, default=8000)
    args = parser.parse_args()
    asyncio.run(main(args.port))
