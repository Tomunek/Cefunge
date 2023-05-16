#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"

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

// Read program from file. Returns 0 on success, otherwise returns non-zero error code
int read_field_from_file(const char *file_name)
{
    FILE *file = fopen(file_name, "r");
    // Detect file opening error
    if (!file)
        return FILE_ERROR_OPEN;

    int max_line_length = field_width + 1;
    char *line = calloc(max_line_length, sizeof(char));
    int line_length = -1;
    int line_number = 0;

    while (line_length != 0)
    {
        memset(line, 0, max_line_length);
        line_length = 0;
        if (fgets(line, max_line_length, file))
        {
            line_length = strlen(line);
        }
        // Detect width overflow
        if (line_length == max_line_length)
        {
            return FILE_ERROR_WIDTH_OVERFLOW;
        }
        for (int i = 0; i < line_length; ++i)
        {
            if (line[i] == '\n' || line[i] == '\r')
                line[i] = ' ';
        }
        // Detect height overflow or insert line into field
        if (line_number < field_height)
        {
            insert_line(line, 0, line_number);
        }
        else
        {
            return FILE_ERROR_HEIGHT_OVERFLOW;
        }
        line_number++;
    }

    fclose(file);
    return FILE_OK;
}

// Calculate one step of a program. Returns -1 if execution continues, otherwise returns non-zero error code
int step_field()
{
    printf("TODO: run step\n");
    // TODO: run step (interpreting instructions)
    return RUNTIME_OK_END;
}

// Run program on field. Returns 0 on succesful exit (@), otherwise returns non-zero error code
int run_field()
{
    int status = RUNTIME_CONTINUE_EXECUTION;
    while (status == RUNTIME_CONTINUE_EXECUTION)
    {
        status = step_field();
    }
    return status;
}

int main()
{
    // TODO: add command line arguments handling
    field_width = 80;
    field_height = 25;
    char* file_name = "examples/HelloWorld.befunge";
    field_length = field_width * field_height;

    // Allocate field
    field = calloc(field_length, sizeof(char));
    if(!field){
        printf("[ERROR] Could not allocate memory for field of size %dx%d (%d bytes)", field_width, field_height, field_length);
        return -1;
    }
    // Set field to all [SPACE]
    memset(field, ' ', field_length);

    int status;
    // Read program from file
    status = read_field_from_file(file_name);

    // Execute rest of the program only if reading was successful
    if(status == FILE_OK){
        print_field(stdout, '-');

        status = run_field();
        printf("Program finished with code: %s\n", error_to_string(status));
    }else{
        // File reading error happened
        printf("[ERROR] Could not read program from file %s: %s\n", file_name, error_to_string(status));
    }

    // Deallocate field
    free(field);
    return status;
}