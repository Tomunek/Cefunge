#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Runtime error codes
#define CONTINUE_EXECUTION -1
#define ERROR_OK_END 0
#define ERROR_STACK_OVERFLOW 1
#define ERROR_STACK_UNDERFLOW 2
#define ERROR_BOUNDRY_REACHED 3
#define ERROR_OUT_OF_RANGE_OP 4
#define ERROR_TIMEOUT 5
// Error code to string translator
static const char *error_to_string(int error)
{
    switch (error)
    {
    case ERROR_OK_END:
        return "Program finished successfully";
    case ERROR_STACK_OVERFLOW:
        return "Stack overflow";
    case ERROR_STACK_UNDERFLOW:
        return "Stack underflow";
    case ERROR_BOUNDRY_REACHED:
        return "Instruction pointer reached playfield boundry";
    case ERROR_OUT_OF_RANGE_OP:
        return "Used out-of-range pointer";
    case ERROR_TIMEOUT:
        return "Program timed out";
    default:
        return "Unknown error code! This message should never be displayed!";
    }
}

// Playfield
char *field;
unsigned int field_width;
unsigned int field_height;
unsigned int field_length;
// Helper function to 2D-ify field
unsigned int coords(unsigned int x, unsigned int y) { return x + y * field_width; }

// Print playfield to selected stream
void print_field(FILE *output_stream, unsigned char decorations)
{
    if (decorations)
    {
        for (int i = 0; i < field_width; ++i)
            putc(decorations, output_stream);
        putc('\n', output_stream);
    }

    for (int i = 0; i < field_height; ++i)
    {
        for (int j = 0; j < field_width; ++j)
        {
            putc(field[coords(j, i)], output_stream);
        }
        putc('\n', output_stream);
    }
    if (decorations)
    {
        for (int i = 0; i < field_width; ++i)
            putc(decorations, output_stream);
        putc('\n', output_stream);
    }
}

// Insert line of code at specified coordinates
void insert_line(const char *line, unsigned int x, unsigned int y)
{
    strcpy(field + field_width * y + x, line);
}

void debug_insert_hello_world()
{
    insert_line(">              v", 0, 0);
    insert_line("v\"Hello World!\"<", 0, 1);
    insert_line(">:v", 0, 2);
    insert_line("^,_@", 0, 3);
}

// Calculate one step of a program. Returns -1 if execution continues, otherwise returns non-zero error code
int step_field()
{
    printf("TODO: run step\n");
    // TODO: run step
    return 0;
}

// Run program on field. Returns 0 on succesful exit (@), otherwise returns non-zero error code
int run_field()
{
    int status = -1;
    while (status < 0)
    {
        status = step_field();
    }
    return status;
}

int main()
{
    field_width = 80;
    field_height = 25;
    field_length = field_width * field_height;

    // Allocate field
    field = calloc(field_length, sizeof(char));
    // Set field to all [SPACE]
    memset(field, ' ', field_length);

    debug_insert_hello_world();

    print_field(stdout, '-');

    int status = run_field();
    printf("Program finished with code: %d (%s)\n", status, error_to_string(status));

    // Deallocate field
    free(field);
    return 0;
}