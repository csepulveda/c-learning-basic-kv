#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../src/protocol.h"

int main() {
    assert(parse_command("PING") == CMD_PING);
    assert(parse_command("SET key=value") == CMD_SET);

    char k[64], v[64];
    assert(extract_key_value("SET foo=bar", k, v, 64, 64) == 0);
    assert(strcmp(k, "foo") == 0 && strcmp(v, "bar") == 0);
    
    assert(extract_key("GET key", k, 64) == 0);
    assert(strcmp(k, "key") == 0);

    printf("✅ Simple protocol tests passed\n");
    return 0;
}