#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define ONE_CHAR 'X'
#define ZERO_CHAR '_'

void to_binary_representation(char* sink, char n)
{
    for (size_t bit = 0; bit < 8; ++bit)
    {
        char c = (n >> bit) & 1 ? ONE_CHAR : ZERO_CHAR;
        sink[7 - bit] = c;
    }
}

bool is_whitespace(char c)
{
    return c == ' '
        || c == '\t'
        || c == '\n';
}

size_t read_lines(char** lines, char* file_buffer, size_t file_buffer_size)
{
    size_t lines_index = 0;
    size_t file_buffer_index = 0;
    char c = file_buffer[file_buffer_index];

    while (file_buffer_index < file_buffer_size)
    {
        char line_buffer[256];
        size_t line_buffer_index = 0;

        while ((c = file_buffer[file_buffer_index++]) != '\n'
               && file_buffer_index < file_buffer_size)
        {
            line_buffer[line_buffer_index++] = c;
        }

        if (line_buffer_index)
        {   // Not a completely empty line.

            char* line = (char*)malloc(256 * sizeof(char));
            if (line)
            {
                // Copy into allocated char array and remove comments.
                size_t line_index = 0;
                while (line_index < line_buffer_index
                       && line_buffer[line_index] != ';')
                {
                    line[line_index] = line_buffer[line_index];

                    // No line breaks in our lines array!
                    assert(line[line_index] != '\n');

                    ++line_index;
                }

                if (line_index)
                {
                    line[line_index] = 0;
                    lines[lines_index++] = line;
                }
                else
                {
                    free(line);
                }
            }
            else
            {
                printf("Memory allocation failure.\n");
            }
        }
    }

    // Remove lines with only whitespace.
    for (size_t i = 0; i < lines_index; ++i)
    {
        bool contains_valid_char = false;
        char* line = lines[i];

        for (size_t j = 0; line[j]; ++j)
        {
            if (line[j] == ONE_CHAR || line[j] == ZERO_CHAR)
            {
                contains_valid_char = true;
            }
        }

        if (!contains_valid_char)
        {
            free(line);

            for (size_t j = i; j < lines_index - 1; ++j)
            {
                lines[j] = lines[j + 1];
            }

            --lines_index;
        }
    }

    return lines_index;
}

int generate_bytecode(char* file_buffer, size_t file_buffer_size)
{
    /* Preprocessing:
        - Remove comments and empty lines
        - Create label map, mapping labels to their instruction addresses
        - Remove labels, whitespace
        - Pad instructions and numeric constants
        - Replace label references with mapped addresses
        - Make sure we end up only with valid characters
    */

    char** lines = (char**)malloc(2048 * sizeof(char*));
    size_t lines_index = 0;

    if (lines)
    {
        lines_index = read_lines(lines, file_buffer, file_buffer_size);

        char labels[256][5];
        char label_instruction_addresses[256][9];
        size_t labels_index = 0;

        // Map labels to their instruction addresses and remove labels.
        for (size_t i = 0; i < lines_index; ++i)
        {
            if (!is_whitespace(lines[i][0]))
            {
                for (size_t j = 0; j < 4; ++j)
                {
                    labels[labels_index][j] = lines[i][j];
                    // Remove label (set to spaces).
                    lines[i][j] = ' ';
                }
                labels[labels_index][4] = 0;

                // Save the line number as the instruction address.
                to_binary_representation(
                        label_instruction_addresses[labels_index],
                        (char)i);
                label_instruction_addresses[labels_index][8] = 0;

                ++labels_index;
            }
        }

        // Remove leading whitespace, pad tokens, replace label references.
        for (size_t i = 0; i < lines_index; ++i)
        {
            char unpadded_token[9];
            size_t token_length = 0;

            for (size_t j = 0, c = lines[i][j]; c; ++j, c = lines[i][j])
            {
                if (!is_whitespace(c))
                {
                    unpadded_token[token_length++] = c;
                }
            }

            unpadded_token[token_length] = 0;

            // Replace token if its a label.
            if (token_length == 4)
            {
                for (size_t j = 0; j < labels_index; ++j)
                {
                    if (strcmp(unpadded_token, labels[j]) == 0)
                    {
                        strcpy(unpadded_token, label_instruction_addresses[j]);
                    }
                }

                // Set token_length
                token_length = 8;
            }

            for (size_t j = 0; j < 8; ++j)
            {
                lines[i][j] = ZERO_CHAR;
            }

            for (size_t j = 0; j < token_length; ++j)
            {
                lines[i][8 - token_length + j] = unpadded_token[j];
            }

            // Pad the numeric constants and label references with a 1 for push.
            if (token_length == 7 || token_length == 8)
            {
                lines[i][0] = ONE_CHAR;
            }

            lines[i][8] = 0;
        }

        /*
        for (int i = 0; i < labels_index; ++i)
        {
            printf("%s : %s\n",
                   labels[i],
                   label_instruction_addresses[i]);
        }
        */

        // Check if everything is valid.
        for (size_t i = 0; i < lines_index; ++i)
        {
            size_t line_index = 0;
            char c;
            while ((c = lines[i][line_index++]))
            {
                if (c != ONE_CHAR && c != ZERO_CHAR)
                {
                    printf("Bytecode generation error!\n Invalid character?\n");
                    return 0;
                }
            }
        }

        for (size_t i = 0; i < lines_index; ++i)
        {
            printf("%s\n", lines[i]);
            free(lines[i]);
        }

        free(lines);

        return 1;
    }
    else
    {
        printf("Memory allocation failure.\n");
        return 0;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s file\n", argv[0]);
        exit(1);
    }

    FILE* file;
    file = fopen(argv[1], "r");

    if (file)
    {
        // Get the filesize. Note: We still want to generate an empty bytecode
        // file when reading an empty input file.
        size_t filesize;
        fseek(file, 0L, SEEK_END);
        filesize = ftell(file);
        // Reset filehandle to the beginning of the file.
        fseek(file, 0L, SEEK_SET);

        // Allocate file buffer
        char* file_buffer = (char*)malloc(filesize);
        if (file_buffer)
        {
            size_t bytes_read = 0;

            while (!feof(file))
            {
                size_t count = fread(file_buffer, sizeof(char), filesize, file);

                if (ferror(file))
                {
                    printf("Error while reading the file.\n");
                }
                else
                {
                    bytes_read += count;
                }
            }

            if (bytes_read == filesize)
            {
                int result = generate_bytecode(file_buffer, bytes_read);
                if (!result)
                {
                    printf("Error during bytecode generation.\n");
                    exit(1);
                }
            }
            else if (filesize == 0)
            {
                // Create empty output file
            }
            else
            {
                printf("Error while reading the file: %d\n", bytes_read);
            }

            free(file_buffer);
        }
        else
        {
            printf("Memory allocation failure.\nCould not allocate %d bytes.\n", filesize);
        }

        // Close the filehandle.
        fclose(file);
    }
    else
    {
        printf("Could not open file %s.\n", argv[1]);
        exit(1);
    }
}
