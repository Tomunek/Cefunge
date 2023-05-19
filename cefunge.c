#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "error.h"

#define DEFAULT_WIDTH 80
#define DEFAULT_HEIGHT 25
#define DEFAULT_STACK_CAPACITY 1024

#define MAX_READABLE_FILE_LINE 256

// Playfield
char *field;
int field_width;
int field_height;
int field_length;
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
void insert_line(const char *line, unsigned int len, unsigned int x, unsigned int y)
{
    memcpy(field + field_width * y + x, line, len);
}

// Read program from file. Returns 0 on success, otherwise returns non-zero error code
int read_field_from_file(const char *file_name)
{
    FILE *file = fopen(file_name, "r");
    // Detect file opening error
    if (!file)
        return FILE_ERROR_OPEN;

    int max_line_length = MAX_READABLE_FILE_LINE;
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
        if(line_length > 0){
            // Detect width overflow
            if (line_length > field_width)
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
                insert_line(line, line_length, 0, line_number);
            }
            else
            {
                return FILE_ERROR_HEIGHT_OVERFLOW;
            }
            line_number++;
        }
    }

    fclose(file);
    return FILE_OK;
}

int ip_x, ip_y;   // Instruction pointer
char ip_inertia;  // Instruction pointer inertia (^/v/</>)
bool string_mode; // Is string mode active
signed long int *stack;
int stack_size;
int stack_capacity;

void stack_push(signed long int v)
{
    stack[stack_size] = v;
    stack_size += 1;
}

signed long int stack_pop(void)
{
    if (stack_size > 0)
    {
        stack_size -= 1;
        return stack[stack_size];
    }
    return 0;
}

void print_debug_info(void)
{
    printf("IP: %d,%d\n", ip_x, ip_y);
    printf("DIR b%c\n", ip_inertia);
    printf("OP: %c (%hhd)\n", field[coords(ip_x, ip_y)], field[coords(ip_x, ip_y)]);
    printf("STACK CONTENTS (size: %d, cap: %d):\n", stack_size, stack_capacity);
    for (int i = 0; i < stack_size; ++i)
    {
        printf("STACK(%d): %ld\n", i, stack[i]);
    }
}

void handle_ip_out_of_field(void)
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
int step_field(void)
{
    if (!string_mode)
    {
        signed long int a, b, x, y, v;
        switch (field[coords(ip_x, ip_y)])
        {
        // Nothing
        case 0:
            break;
        case ' ':
            break;
        // Addition: Pop two values a and b, then push the result of a+b
        case '+':
            a = stack_pop();
            b = stack_pop();
            stack_push(a + b);
            break;
        // Subtraction: Pop two values a and b, then push the result of b-a
        case '-':
            a = stack_pop();
            b = stack_pop();
            stack_push(b - a);
            break;
        // Multiplication: Pop two values a and b, then push the result of a*b
        case '*':
            a = stack_pop();
            b = stack_pop();
            stack_push(a * b);
            break;
        // Integer division: Pop two values a and b, then push the result of b/a, rounded down.
        // According to the specifications, if a is zero, ask the user what result they want.
        case '/':
            a = stack_pop();
            b = stack_pop();
            if (a != 0)
            {
                stack_push(b / a);
            }
            else
            {
                // Division by 0
                printf("Division by 0 occured. Enter desired result (or 0 will be assumed): ");
                if (scanf("%ld", &a) == EOF)
                {
                    stack_push(a);
                }
            }
            break;
        // Modulo: Pop two values a and b, then push the remainder of the integer division of b/a.
        case '%':
            a = stack_pop();
            b = stack_pop();
            stack_push(b % a);
            break;
        // Logical NOT: Pop a value. If the value is zero, push 1; otherwise, push zero.
        case '!':
            a = stack_pop();
            stack_push(!a);
            break;
        // Greater than: Pop two values a and b, then push 1 if b>a, otherwise zero.
        case '`':
            a = stack_pop();
            b = stack_pop();
            if (b > a)
            {
                stack_push(1);
            }
            else
            {
                stack_push(0);
            }
            break;
        // PC direction right
        case '>':
            ip_inertia = '>';
            break;
        // PC direction left
        case '<':
            ip_inertia = '<';
            break;
        // PC direction up
        case '^':
            ip_inertia = '^';
            break;
        // PC direction down
        case 'v':
            ip_inertia = 'v';
            break;
        // Random PC direction
        case '?':
            switch (rand() % 4)
            {
            case 0:
                ip_inertia = '>';
                break;
            case 1:
                ip_inertia = '<';
                break;
            case 2:
                ip_inertia = '^';
                break;
            case 3:
                ip_inertia = 'v';
                break;
            }
            break;
        // Horizontal IF: pop a value; set direction to right if value=0, set to left otherwise
        case '_':
            a = stack_pop();
            if (a == 0)
            {
                ip_inertia = '>';
            }
            else
            {
                ip_inertia = '<';
            }
            break;
        // Vertical IF: pop a value; set direction to down if value=0, set to up otherwise
        case '|':
            a = stack_pop();
            if (a == 0)
            {
                ip_inertia = '^';
            }
            else
            {
                ip_inertia = 'v';
            }
            break;
        // Toggle stringmode (push each character's ASCII value all the way up to the next ")
        case '"':
            string_mode = true;
            break;
        // Duplicate top stack value
        case ':':
            a = stack_pop();
            stack_push(a);
            stack_push(a);
            break;
        // Swap top stack values
        case '\\':
            a = stack_pop();
            b = stack_pop();
            stack_push(a);
            stack_push(b);
            break;
        // Pop top of stack and discard
        case '$':
            a = stack_pop();
            break;
        // Pop top of stack and output as integer
        case '.':
            a = stack_pop();
            printf("%ld", a);
            break;
        // Pop top of stack and output as ASCII character
        case ',':
            a = stack_pop();
            printf("%c", (char)a);
            break;
        // Bridge: jump over next command in the current direction of the current PC
        case '#':
            switch (ip_inertia)
            {
            case '^':
                ip_y -= 1;
                break;
            case 'v':
                ip_y += 1;
                break;
            case '<':
                ip_x -= 1;
                break;
            case '>':
                ip_x += 1;
                break;
            }
            handle_ip_out_of_field();
            break;
        // A "get" call (a way to retrieve data in storage). Pop two values y and x, then push the ASCII value
        // of the character at that position in the program. If (x,y) is out of bounds, push 0
        case 'g':
            y = stack_pop();
            x = stack_pop();
            if (x < 0 || x >= field_width || y < 0 || y >= field_height)
            {
                stack_push(0);
            }
            else
            {
                stack_push(field[coords(x, y)]);
            }
            break;
        // A "put" call (a way to store a value for later use). Pop three values y, x and v, then change the
        // character at the position (x,y) in the program to the character with ASCII value v
        case 'p':
            y = stack_pop();
            x = stack_pop();
            v = stack_pop();
            if (x < 0 || x >= field_width || y < 0 || y >= field_height)
            {
                return RUNTIME_ERROR_OUT_OF_RANGE_OP;
            }
            else
            {
                field[coords(x, y)] = v;
            }
            break;
        // Get integer from user and push it
        case '&':
            if (scanf("%ld", &a) == EOF)
            {
                a = 0;
            }
            stack_push(a);
            break;
        // Get character from user and push it
        case '~':
            if (scanf("%c", (char *)&a) == EOF)
            {
                a = 0;
            }
            stack_push(a);
            break;
        // End program
        case '@':
            return RUNTIME_OK_END;
            break;
        default:
            // Push corresponding number onto the stack
            if ('0' <= field[coords(ip_x, ip_y)] && field[coords(ip_x, ip_y)] <= '9')
            {
                stack_push(field[coords(ip_x, ip_y)] - '0');
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
            stack_push(field[coords(ip_x, ip_y)]);
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
    case '^':
        ip_y -= 1;
        break;
    case 'v':
        ip_y += 1;
        break;
    case '<':
        ip_x -= 1;
        break;
    case '>':
        ip_x += 1;
        break;
    }
    handle_ip_out_of_field();

    return RUNTIME_CONTINUE_EXECUTION;
}

// Run program on field. Returns 0 on succesful exit (@), otherwise returns non-zero error code
int run_field(void)
{
    int status = RUNTIME_CONTINUE_EXECUTION;
    while (status == RUNTIME_CONTINUE_EXECUTION)
    {
        // print_debug_info();
        status = step_field();
    }
    return status;
}

void print_usage(void)
{
    printf("Usage:\n");
    printf("cefunge [-w <width>] [-h <height>] [-s <stack capacity>] program.befunge\n");
    printf("Default: width %d, height %d, stack capacity %d\n", DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STACK_CAPACITY);
}

int main(int argc, char **argv)
{
    // Usage:
    // cefunge [-w 80] [-h 25] [-s 1024] program.befunge
    // -w width     [80]
    // -h height    [25]
    // -s stack cap [1024]
    // program name as last argument

    // Argument handling
    if (argc < 2)
    {
        printf("[ERROR] Befunge file not specified\n");
        print_usage();
        return -1;
    }

    // Default values
    field_width = DEFAULT_WIDTH;
    field_height = DEFAULT_HEIGHT;
    stack_capacity = DEFAULT_STACK_CAPACITY;

    int current_arg = 1;
    while(current_arg < argc - 2){
        if(strcmp(argv[current_arg], "-w") == 0){
            current_arg += 1;
            field_width = atoi(argv[current_arg]);
            if(field_width <= 0){
                printf("[ERROR] Invalid field width specified (%s)!\n", argv[current_arg]);
                print_usage();
                return -1;
            }
            current_arg += 1;
        }

        if(strcmp(argv[current_arg], "-h") == 0){
            current_arg += 1;
            field_height = atoi(argv[current_arg]);
            if(field_height <= 0){
                printf("[ERROR] Invalid field height specified (%s)!\n", argv[current_arg]);
                print_usage();
                return -1;
            }
            current_arg += 1;
        }

        if(strcmp(argv[current_arg], "-s") == 0){
            current_arg += 1;
            stack_capacity = atoi(argv[current_arg]);
            if(stack_capacity <= 0){
                printf("[ERROR] Invalid stack capacity specified (%s)!\n", argv[current_arg]);
                print_usage();
                return -1;
            }
            current_arg += 1;
        }
    }

    char *file_name = argv[argc - 1];

    // Seed random
    srand(time(0));

    // Allocate field
    field_length = field_width * field_height;
    field = calloc(field_length, sizeof(char));
    if (!field)
    {
        printf("[ERROR] Could not allocate memory for field of size %dx%d (%d bytes)", field_width, field_height, field_length);
        return -1;
    }
    // Set field to all [SPACE]
    memset(field, ' ', field_length);

    // Read program from file
    int status;
    status = read_field_from_file(file_name);

    // Execute rest of the program only if reading was successful
    if (status == FILE_OK)
    {
        // Allocate stack
        stack = calloc(stack_capacity, sizeof(signed long int));
        stack_size = 0;
        memset(stack, 0, stack_capacity);

        ip_x = 0;
        ip_y = 0;
        ip_inertia = '>';
        string_mode = false;

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