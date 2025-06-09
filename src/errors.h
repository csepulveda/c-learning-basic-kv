#ifndef ERRORS_H
#define ERRORS_H

#define ERR_KEY_TOO_LONG   "ERROR key too long\n"
#define ERR_VALUE_TOO_LONG "ERROR value too long\n"
#define ERR_NOT_FOUND      "ERROR not found\n"
#define ERR_PARSE_ERROR    "ERROR parse error\n"
#define ERR_UNKNOWN_CMD    "ERROR unknown command\n"
#define ERR_INTERNAL_ERROR "ERROR internal error\n"

#define EXTRACT_OK                0
#define EXTRACT_ERR_PARSE        -1
#define EXTRACT_ERR_KEY_TOO_LONG -2
#define EXTRACT_ERR_VALUE_TOO_LONG -3
#define EXTRACT_ERR_KEY_NOT_FOUND -4
#define EXTRACT_ERR_INTERNAL     -5

#endif
