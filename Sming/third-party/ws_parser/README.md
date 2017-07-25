# ws_parser

ws_parser is a streaming parser for the WebSocket protocol ([RFC 6455](https://tools.ietf.org/html/rfc6455)).

This library is loosely inspired by [joyent/http_parser](https://github.com/joyent/http-parser) and shares many of the same attributes: it has no dependencies, makes no allocations or syscalls, and only requires a very small amount of memory to maintain its parse state: 30 bytes on 64 bit systems (or 22 bytes on 32 bit systems).
