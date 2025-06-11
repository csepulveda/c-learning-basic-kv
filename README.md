[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=csepulveda_c-learning-basic-kv&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=csepulveda_c-learning-basic-kv)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=csepulveda_c-learning-basic-kv&metric=coverage)](https://sonarcloud.io/summary/new_code?id=csepulveda_c-learning-basic-kv)
# Simple Key-Value Store Server & Client

This is a basic educational project implementing a simple key-value store server and client, written in C.

The project supports basic commands similar to Redis:

- `PING` — respond with `PONG`
- `TIME` — return current server time
- `SET key value` — store a key-value pair
- `GET key` — retrieve the value of a key
- `DEL key` — delete a key
- `MSET key1 value1 key2 value2 ...` — store multiple key-value pairs
- `MGET key1 key2 ...` — retrieve values of multiple keys
- `TYPE KEY1` - retrive the Type of the value, eg: string
- `INFO`  - Information about the server.

## Project Structure

- `src/server.c` — server implementation
- `src/client.c` — client implementation
- `src/commands.c` — command handlers
- `src/protocol.c` — command parsing
- `src/kvstore.c` — in-memory key-value store
- `src/logs.c` — simple logging
- `src/client_utils.c` — utilities for the client
- `tests/` — unit tests

## Building

```bash
make
```

## Running

Start the server:

```bash
./bin/server
```

In another terminal, run the client:

```bash
./bin/client PING
./bin/client SET foo bar
./bin/client GET foo
./bin/client DEL foo
./bin/client MSET key1 value1 key2 value2
./bin/client MGET key1 key2
./bin/client TIME
./bin/client TYPE key1
./bin/client INFO
```

## Testing

Run unit tests:

```bash
make test
```

Run Integration tests:
```bash
make integration-test
```

## Notes
- All data is kept in memory and is not persistent.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
