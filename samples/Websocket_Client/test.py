#!/usr/bin/env python
#
# Test python application to send test message to server
#
# Shows connection detail so we can compare it to Sming code
#

import argparse
import logging
import asyncio
from websockets.sync.client import connect

def main():
    parser = argparse.ArgumentParser(description='Simple websocket client test')
    parser.add_argument('URL', help='Connection URL', default='ws://192.168.13.10/ws')

    args = parser.parse_args()

    logging.basicConfig(
        format="%(asctime)s %(message)s",
        level=logging.DEBUG,
    )
    print(f'Connecting to {args.URL}...')
    with connect(args.URL) as websocket:
        websocket.send("Hello world!")
        websocket.recv()
        websocket.recv()

if __name__ == "__main__":
    main()
