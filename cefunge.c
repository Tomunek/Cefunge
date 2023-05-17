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

int ip_x, ip_y;   // Instruction pointer
char ip_inertia;  // Instruction pointer inertia (u/d/l/r)
bool string_mode; // Is string mode active
signed long int *stack;
int stack_size;
int stack_capacity;

void print_debug_info()
{
    printf("IP:%d,%d\n", ip_x, ip_y);
    printf("DIR:%c\n", ip_inertia);
    printf("OP:%c (%hhd)\n", field[coords(ip_x, ip_y)], field[coords(ip_x, ip_y)]);
    printf("STACK:\n");
    for (int i = 0; i < stack_size; ++i)
    {
        printf("STACK(%d): %ld\n", i, stack[i]);
    }
}

void handle_stack_underflow()
{
    stack[stack_size] = 0;
    stack_size += 1;
}

void handle_ip_out_of_field()
{
    if (ip_x < 0)
    {
        ip_x = field_width - 1;
    }
    if (ip_x >= field_width)
    {
        ip_x = 0;
    }
    if (ip_y < 0)
    {
        ip_y = field_height - 1;
    }
    if (ip_y >= field_height)
    {
        ip_y = 0;
    }
}

// Calculate one step of a program. Returns -1 if execution continues, otherwise returns non-zero error code
int step_field()
{
    if (!string_mode)
    {
        switch (field[coords(ip_x, ip_y)])
        {
        // Nothing
        case 0:
            break;
        case ' ':
            break;
        // Addition: Pop two values a and b, then push the result of a+b
        case '+':
            if (stack_size >= 2)
            {
                stack[stack_size - 2] = stack[stack_size - 2] + stack[stack_size - 1];
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Subtraction: Pop two values a and b, then push the result of b-a
        case '-':
            if (stack_size >= 2)
            {
                stack[stack_size - 2] = stack[stack_size - 2] - stack[stack_size - 1];
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Multiplication: Pop two values a and b, then push the result of a*b
        case '*':
            if (stack_size >= 2)
            {
                stack[stack_size - 2] = stack[stack_size - 2] * stack[stack_size - 1];
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Integer division: Pop two values a and b, then push the result of b/a, rounded down.
        // According to the specifications, if a is zero, ask the user what result they want.
        case '/':
            if (stack_size >= 2)
            {
                if (stack[stack_size - 1] != 0)
                {
                    stack[stack_size - 2] = stack[stack_size - 2] / stack[stack_size - 1];
                }
                else
                {
                    // Division by 0
                    printf("Division by 0 occured. Enter desired result (or 0 will be assumed): ");
                    if (scanf("%ld", &stack[stack_size - 2]) == EOF)
                    {
                        stack[stack_size - 2] = 0;
                    }
                }
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Modulo: Pop two values a and b, then push the remainder of the integer division of b/a.
        case '%':
            if (stack_size >= 2)
            {
                stack[stack_size - 2] = stack[stack_size - 2] % stack[stack_size - 1];
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Logical NOT: Pop a value. If the value is zero, push 1; otherwise, push zero.
        case '!':
            if (stack_size >= 1)
            {
                stack[stack_size - 1] = !stack[stack_size - 1];
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Greater than: Pop two values a and b, then push 1 if b>a, otherwise zero.
        case '`':
            if (stack_size >= 2)
            {
                if (stack[stack_size - 2] > stack[stack_size - 1])
                {
                    stack[stack_size - 2] = 1;
                }
                else
                {
                    stack[stack_size - 2] = 0;
                }
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // PC direction right
        case '>':
            ip_inertia = 'r';
            break;
        // PC direction left
        case '<':
            ip_inertia = 'l';
            break;
        // PC direction up
        case '^':
            ip_inertia = 'u';
            break;
        // PC direction down
        case 'v':
            ip_inertia = 'd';
            break;
        // Random PC direction
        case '?':
            switch (rand() % 4)
            {
            case 0:
                ip_inertia = 'r';
                break;
            case 1:
                ip_inertia = 'l';
                break;
            case 2:
                ip_inertia = 'u';
                break;
            case 3:
                ip_inertia = 'd';
                break;
            }
            break;
        // Horizontal IF: pop a value; set direction to right if value=0, set to left otherwise
        case '_':
            if (stack_size >= 1)
            {
                if (stack[stack_size - 1] == 0)
                {
                    ip_inertia = 'r';
                }
                else
                {
                    ip_inertia = 'l';
                }
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Vertical IF: pop a value; set direction to down if value=0, set to up otherwise
        case '|':
            if (stack_size >= 1)
            {
                if (stack[stack_size - 1] == 0)
                {
                    ip_inertia = 'd';
                }
                else
                {
                    ip_inertia = 'u';
                }
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Toggle stringmode (push each character's ASCII value all the way up to the next ")
        case '"':
            string_mode = true;
            break;
        // Duplicate top stack value
        case ':':
            if (stack_size >= 1)
            {
                stack[stack_size] = stack[stack_size - 1];
                stack_size += 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Swap top stack values
        case '\\':
            if (stack_size >= 2)
            {
                signed char tmp = stack[stack_size - 2];
                stack[stack_size - 2] = stack[stack_size - 1];
                stack[stack_size - 1] = tmp;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Pop top of stack and discard
        case '$':
            if (stack_size >= 1)
            {
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Pop top of stack and output as integer
        case '.':
            if (stack_size >= 1)
            {
                printf("%d", (short int)stack[stack_size - 1]);
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Pop top of stack and output as ASCII character
        case ',':
            if (stack_size >= 1)
            {
                printf("%c", (char)stack[stack_size - 1]);
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Bridge: jump over next command in the current direction of the current PC
        case '#':
            switch (ip_inertia)
            {
            case 'u':
                ip_y -= 1;
                break;
            case 'd':
                ip_y += 1;
                break;
            case 'l':
                ip_x -= 1;
                break;
            case 'r':
                ip_x += 1;
                break;
            }
            handle_ip_out_of_field();
            break;
        // A "get" call (a way to retrieve data in storage). Pop two values y and x, then push the ASCII value
        // of the character at that position in the program. If (x,y) is out of bounds, push 0
        case 'g':
            if (stack_size >= 2)
            {
                int y = stack[stack_size - 1];
                int x = stack[stack_size - 2];
                if (x < 0 || x >= field_width || y < 0 || y >= field_height)
                {
                    stack[stack_size - 2] = 0;
                }
                else
                {
                    stack[stack_size - 2] = field[coords(x, y)];
                }
                stack_size -= 1;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // A "put" call (a way to store a value for later use). Pop three values y, x and v, then change the
        // character at the position (x,y) in the program to the character with ASCII value v
        case 'p':
            if (stack_size >= 3)
            {
                int y = stack[stack_size - 1];
                int x = stack[stack_size - 2];
                int v = stack[stack_size - 3];
                if (x < 0 || x >= field_width || y < 0 || y >= field_height)
                {
                    return RUNTIME_ERROR_OUT_OF_RANGE_OP;
                }
                else
                {
                    field[coords(x, y)] = v;
                }
                stack_size -= 3;
            }
            else
            {
                handle_stack_underflow();
            }
            break;
        // Get integer from user and push it
        case '&':
            if (scanf("%ld", &stack[stack_size]) == EOF)
            {
                stack[stack_size] = 0;
            }
            stack_size += 1;
            break;
        // Get character from user and push it
        case '~':
            if (scanf("%c", (char *)&stack[stack_size]) == EOF)
            {
                stack[stack_size] = 0;
            }
            stack_size += 1;
            break;
        // End program
        case '@':
            return RUNTIME_OK_END;
            break;
        default:
            // Push corresponding number onto the stack
            if ('0' <= field[coords(ip_x, ip_y)] && field[coords(ip_x, ip_y)] <= '9')
            {
                stack[stack_size] = field[coords(ip_x, ip_y)] - '0';
                stack_size += 1;
            }
            else
            {
                return RUNTIME_ERROR_UNKNOWN_OP;
            }
        }
    }
    else
    {
        // String mode
        if (field[coords(ip_x, ip_y)] != '"')
        {
            stack[stack_size] = field[coords(ip_x, ip_y)];
            stack_size++;
        }
        else
        {
            string_mode = false;
        }
    }

    if (stack_size >= stack_capacity - 1)
    {
        return RUNTIME_ERROR_STACK_OVERFLOW;
    }

    switch (ip_inertia)
    {
    case 'u':
        ip_y -= 1;
        break;
    case 'd':
        ip_y += 1;
        break;
    case 'l':
        ip_x -= 1;
        break;
    case 'r':
        ip_x += 1;
        break;
    }
    handle_ip_out_of_field();

    return RUNTIME_CONTINUE_EXECUTION;
}

// Run program on field. Returns 0 on succesful exit (@), otherwise returns non-zero error code
int run_field()
{
    int status = RUNTIME_CONTINUE_EXECUTION;
    while (status == RUNTIME_CONTINUE_EXECUTION)
    {
        status = step_field();
        print_debug_info();
    }
    return status;
}

int main()
{
    // TODO: add command line arguments handling
    field_width = 80;
    field_height = 25;
    char *file_name = "examples/Calculator.befunge";
    field_length = field_width * field_height;

    // Allocate field
    field = calloc(field_length, sizeof(char));
    if (!field)
    {
        printf("[ERROR] Could not allocate memory for field of size %dx%d (%d bytes)", field_width, field_height, field_length);
        return -1;
    }
    // Set field to all [SPACE]
    memset(field, ' ', field_length);

    int status;
    // Read program from file
    status = read_field_from_file(file_name);

    string_mode = false;

    // Execute rest of the program only if reading was successful
    if (status == FILE_OK)
    {
        // Allocate stack
        stack_capacity = 1024;
        stack = calloc(stack_capacity, sizeof(signed long int));
        stack_size = 0;
        memset(stack, 0, stack_capacity);

        ip_x = 0;
        ip_y = 0;
        ip_inertia = 'r';

        print_field(stdout, '-');

        status = run_field();
        printf("\n\nProgram finished with code: %s\n", error_to_string(status));
        if (status != RUNTIME_OK_END)
        {
            printf("[ERROR]: %s\n", error_to_string(status));
            print_debug_info();
        }

        free(stack);
    }
    else
    {
        // File reading error happened
        printf("[ERROR] Could not read program from file %s: %s\n", file_name, error_to_string(status));
    }

    // Deallocate field
    free(field);
    return status;
}