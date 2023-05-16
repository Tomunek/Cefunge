#include "error.h"

// Error code to string translator
const char *error_to_string(int error)
{
    switch (error)
    {
    case RUNTIME_OK_END:
        return "Success";
    case RUNTIME_ERROR_STACK_OVERFLOW:
        return "Stack overflow";
    case RUNTIME_ERROR_STACK_UNDERFLOW:
        return "Stack underflow";
    case RUNTIME_ERROR_BOUNDRY_REACHED:
        return "Instruction pointer reached playfield boundry";
    case RUNTIME_ERROR_OUT_OF_RANGE_OP:
        return "Used out-of-range pointer";
    case RUNTIME_ERROR_TIMEOUT:
        return "Program timed out";
    case FILE_ERROR_OPEN:
        return "File opening error";
    case FILE_ERROR_WIDTH_OVERFLOW:
        return "Width overflow";
    case FILE_ERROR_HEIGHT_OVERFLOW:
        return "Height overflow";
    default:
        return "Unknown error code! This message should never be displayed!";
    }
}