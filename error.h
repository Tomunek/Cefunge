#ifndef ERROR_H
#define ERROR_H

// Runtime error codes
#define RUNTIME_CONTINUE_EXECUTION -1
#define RUNTIME_OK_END 0
#define RUNTIME_ERROR_STACK_OVERFLOW 1
#define RUNTIME_ERROR_OUT_OF_RANGE_OP 2
#define RUNTIME_ERROR_UNKNOWN_OP 3
#define RUNTIME_ERROR_TIMEOUT 4

// File error codes
#define FILE_OK 0
#define FILE_ERROR_OPEN 101
#define FILE_ERROR_WIDTH_OVERFLOW 102
#define FILE_ERROR_HEIGHT_OVERFLOW 103

// Error code to string translator
const char *error_to_string(int error);

#endif // ERROR_H