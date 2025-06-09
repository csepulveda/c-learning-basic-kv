# Design of c-learning-basic-kv

## Objective

Simulate a basic key-value database similar to Redis for educational purposes.

## Structure

- `server.c`: Main loop, socket and command dispatch.
- `client.c`: TCP client logic.
- `commands.c`: Logic for each command.
- `kv_store.c`: In-memory storage.
- `protocol.c`: Response helpers.
- `client_utils.c`: Line-by-line `recv()` handling.
- `tests/`: Automated command tests.

## Client response handling

- `recv()` reads byte by byte with `handle_char()` to support fragmentation.
- Lines are accumulated until `\n`.
- `END` is detected to stop reading.

## Possible improvements

- Support for key expiration.
- Support for data types (lists, hashes).
- Use of epoll or select for better performance.