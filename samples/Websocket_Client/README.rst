Websocket Client
================

This is a simple demo of the WebsocketClient class.
It shows connection, closing and reconnection methods of WebsocketClient.

The client tries to connect to a websocket echo server.
It sents 10 messages then client connection is closed.
This sequence repeats after 20 seconds.

The sample was originally written to communicate with *echo.websocket.org*
but that service no longer exists.
Instead, run ``make wsserver`` to run a local test server.
This has the advantage of showing detailed diagnostic information which may be helpful.

Build with ``WS_URL`` set to the server address. For example:

    make WS_URL=ws://192.168.1.10:8000
