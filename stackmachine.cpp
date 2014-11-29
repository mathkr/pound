#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1024
#define REGISTERS 8
#define PROGRAM_BUFFER_SIZE 2048

// Instructions
enum {
    NOP,
    POP,
    ADD,
    SUB,
    LDR,
    WTR,
    JMP,
    OUT,
};

typedef unsigned char int8;

int8* stack;
int8* registers;

char** program;
size_t program_length;

// Stackpointer
size_t sp = 0;
size_t instruction_pointer = 0;

int8 get_numeric_value(char* token)
{
    int8 result = 0;

    // Don't set bit 7 as it is always 1 to indicate a numeric constant.
    for (size_t bit = 0; bit < 7; ++bit)
    {
        bool bit_is_set = token[7 - bit] == '#';

        if (bit_is_set)
        {
            result |= (1 << bit);
        }
    }

    return result;
}

void process_token(char* token)
{
    bool is_numeric_constant = token[0] == '#';
    if (is_numeric_constant)
    {
        int8 value = get_numeric_value(token);

#ifdef DEBUG
        printf("debug: number: %d\n", value);
#endif // DEBUG

        // Push onto the stack.
        stack[sp++] = value;

        // Increment instructionpointer.
        ++instruction_pointer;
    }
    else
    {
        int8 instruction = get_numeric_value(token);

#ifdef DEBUG
        printf("debug: instruction: %d\n", instruction);
#endif // DEBUG

        switch(instruction)
        {
            case NOP: {
                {}
                ++instruction_pointer;
            } break;

            case POP: {
                --sp;
                ++instruction_pointer;
            } break;

            case ADD: {
                stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
                --sp;
                ++instruction_pointer;
            } break;

            case SUB: {
                stack[sp - 2] = stack[sp - 2] - stack[sp - 1];
                --sp;
                ++instruction_pointer;
            } break;

            case LDR: {
                stack[sp - 1] = registers[stack[sp - 1]];
                ++instruction_pointer;
            } break;

            case WTR: {
                registers[stack[sp - 1]] = stack[sp - 2];
                sp -= 2;
                ++instruction_pointer;
            } break;

            case JMP: {
                instruction_pointer = (stack[sp - 2] == 0)
                        ? stack[sp - 1]
                        : instruction_pointer + 1;
                sp -= 2;
            } break;

            case OUT: {

#ifdef DEBUG
                printf("debug: printing: %d\n", stack[sp - 1]);
#endif // DEBUG

                printf("%c", stack[sp - 1]);
                --sp;
                ++instruction_pointer;
            } break;

            default: {
                // Should never happen!
                printf("Error: unknown instruction.\n");
                exit(EXIT_FAILURE);
            } break;
        }

    }
}

void run_program()
{
#ifdef DEBUG
    // Debug output:
    printf("debug: program_length: %d\n", program_length);
#endif // DEBUG

    while (instruction_pointer < program_length)
    {

#ifdef DEBUG
        // Debug output:
        printf("######### begin debug output\n");

        // Stack
        printf("Stack:\n");
        for (size_t i = 0; i < sp; ++i)
        {
            printf("%d\n", stack[i]);
        }

        // Registers
        printf("Registers:\n");
        for (size_t i = 0; i < REGISTERS; ++i)
        {
            printf("%d: %d\n", i, registers[i]);
        }

        // Instruction pointer
        printf("instruction_pointer: %d\n", instruction_pointer);

        // Stackpointer
        printf("stackpointer: %d\n", sp);
        printf("######### end debug output\n");
#endif // DEBUG

        process_token(program[instruction_pointer]);
    }
}

int main(int argc, char* argv[])
{
    // Intialization
    stack = (int8*)malloc(sizeof(int8) * STACK_SIZE);
    registers = (int8*)malloc(sizeof(int8) * REGISTERS);
    program = (char**)malloc(sizeof(char*) * PROGRAM_BUFFER_SIZE);
    program_length = 0;

    if (stack && registers)
    {
        char* line = NULL;
        size_t buffer_size = 0;
        size_t read_size = 0;
        while((read_size = getline(&line, &buffer_size, stdin)) != -1)
        {
            program[program_length] = (char*)malloc(sizeof(char) * read_size);
            strncpy(program[program_length], line, read_size);
            ++program_length;
        }

        run_program();

        if (line) free(line);

        for (size_t i = 0; i < program_length; ++i)
        {
            free(program[i]);
        }
        free(program);

        free(registers);
        free(stack);

        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Memory allocation failure.\n");
        exit(EXIT_FAILURE);
    }
}
