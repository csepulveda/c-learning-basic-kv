#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../src/protocol.h"

/**
 * @brief Runs tests to validate the protocol command parsing and extraction functions.
 *
 * Executes assertions to verify correct behavior of `parse_command`, `extract_key_value`, and `extract_key` for various command inputs. Prints a confirmation message if all tests pass.
 *
 * @return int Returns 0 upon successful completion of all tests.
 */
int main() {
    assert(parse_command("SET key value") == CMD_SET);
    assert(parse_command("PING") == CMD_PING);
    assert(parse_command("PING ") == CMD_PING);
    assert(parse_command("PING\n") == CMD_PING);

    assert(parse_command("GOODBYE") == CMD_GOODBYE);
    assert(parse_command("GOODBYE ") == CMD_GOODBYE);
    assert(parse_command("GOODBYE\n") == CMD_GOODBYE);

    char k[64], v[64];
    assert(extract_key_value("SET foo bar", k, v, 64, 64) == 0);
    assert(strcmp(k, "foo") == 0 && strcmp(v, "bar") == 0);
    
    assert(extract_key("GET key", k, 64) == 0);
    assert(strcmp(k, "key") == 0);

    printf("✅ Simple protocol tests passed\n");
    return 0;
}
