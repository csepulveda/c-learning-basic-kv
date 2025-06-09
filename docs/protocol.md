# Communication Protocol

## Server Response Format

```
RESPONSE <TYPE>
<data>
...
END
```

### Examples:

#### Simple Response
```
RESPONSE OK STRING
OK
END
```

#### Multiple Response (`MGET`)
```
RESPONSE OK MULTI
1) value1
2) value2
3) (nil)
END
```

#### Error Response
```
RESPONSE ERROR
ERROR key too long
END
```

## Design Reasoning

- ✅ Easy to parse with tools like `telnet`.
- ✅ Allows accumulation of data from `recv()`.
- ✅ Extensible for new types (`LIST`, `BULK`, etc).