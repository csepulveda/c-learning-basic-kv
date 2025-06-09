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

## Data Structure

```mermaid
graph TD
    A[hash_table[HASH_TABLE_SIZE]] --> B0[Bucket 0]
    A --> B1[Bucket 1]
    A --> B2[Bucket 2]
    A --> B3[Bucket 3]
    A --> Bn[Bucket N-1]

    subgraph Bucket 0 [Bucket 0 (hash(key) == 0)]
        B0 --> K0A["kv_node(key='foo', value='bar')"] --> K0B["kv_node(key='baz', value='qux')"] --> Null0[(null)]
    end

    subgraph Bucket 1 [Bucket 1 (hash(key) == 1)]
        B1 --> Null1[(null)]
    end

    subgraph Bucket 2 [Bucket 2 (hash(key) == 2)]
        B2 --> K2A["kv_node(key='hello', value='world')"] --> Null2[(null)]
    end

    subgraph Bucket N-1 [Bucket N-1 (hash(key) == N-1)]
        Bn --> NullN[(null)]
    end

    style Null0 fill:#f0f0f0,stroke:#bbb
    style Null1 fill:#f0f0f0,stroke:#bbb
    style Null2 fill:#f0f0f0,stroke:#bbb
    style NullN fill:#f0f0f0,stroke:#bbb
```


## Client response handling

- `recv()` reads byte by byte with `handle_char()` to support fragmentation.
- Lines are accumulated until `\n`.
- `END` is detected to stop reading.

## Possible improvements

- Support for key expiration.
- Support for data types (lists, hashes).
- Use of epoll or select for better performance.