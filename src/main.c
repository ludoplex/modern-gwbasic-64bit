#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/gwbasic.h"

void print_banner(void) {
    printf("Modern GW-BASIC 64-bit Interpreter v0.1\n");
    printf("64-bit architecture with assembly-optimized core\n");
    #ifdef __x86_64__
    printf("Architecture: AMD64 (x86_64)\n");
    #elif defined(__aarch64__)
    printf("Architecture: ARM64 (AArch64)\n");
    #else
    printf("Architecture: Unknown\n");
    #endif
    printf("\n");
}

void print_help(void) {
    printf("Commands:\n");
    printf("  [line#] statement  - Add/replace program line\n");
    printf("  LIST              - List program\n");
    printf("  RUN               - Run program\n");
    printf("  NEW               - Clear program\n");
    printf("  QUIT/EXIT         - Exit interpreter\n");
    printf("  HELP              - Show this help\n");
    printf("\n");
    printf("Statements:\n");
    printf("  PRINT expression  - Print value\n");
    printf("  LET var = value   - Assign variable\n");
    printf("  var = value       - Assign variable (implicit LET)\n");
    printf("  END               - End program\n");
    printf("\n");
}

void list_program(Program *prog) {
    BasicLine *line = prog->first_line;
    while (line) {
        printf("%d %s\n", line->line_num, line->text);
        line = line->next;
    }
}

int main(int argc, char *argv[]) {
    print_banner();
    
    Program *prog = program_new();
    if (!prog) {
        fprintf(stderr, "Failed to initialize program\n");
        return 1;
    }
    
    // Test assembly functions
    int64_t a = 10, b = 5;
    printf("Testing assembly core:\n");
    printf("  %lld + %lld = %lld\n", (long long)a, (long long)b, (long long)asm_add_int(a, b));
    printf("  %lld - %lld = %lld\n", (long long)a, (long long)b, (long long)asm_sub_int(a, b));
    printf("  %lld * %lld = %lld\n", (long long)a, (long long)b, (long long)asm_mul_int(a, b));
    printf("  %lld / %lld = %lld\n", (long long)a, (long long)b, (long long)asm_div_int(a, b));
    
    double da = 10.5, db = 2.5;
    printf("  %.2f + %.2f = %.2f\n", da, db, asm_add_double(da, db));
    printf("  %.2f - %.2f = %.2f\n", da, db, asm_sub_double(da, db));
    printf("  %.2f * %.2f = %.2f\n", da, db, asm_mul_double(da, db));
    printf("  %.2f / %.2f = %.2f\n", da, db, asm_div_double(da, db));
    printf("\n");
    
    // If a file was provided, load it
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (f) {
            char line[1024];
            while (fgets(line, sizeof(line), f)) {
                // Remove newline
                size_t len = strlen(line);
                if (len > 0 && line[len-1] == '\n') {
                    line[len-1] = '\0';
                }
                
                // Parse line number
                int line_num = 0;
                char *ptr = line;
                while (isspace(*ptr)) ptr++;
                
                if (isdigit(*ptr)) {
                    line_num = atoi(ptr);
                    while (isdigit(*ptr)) ptr++;
                    while (isspace(*ptr)) ptr++;
                    program_add_line(prog, line_num, ptr);
                }
            }
            fclose(f);
            
            printf("Program loaded. Running...\n\n");
            program_run(prog);
            program_free(prog);
            return 0;
        } else {
            fprintf(stderr, "Could not open file: %s\n", argv[1]);
        }
    }
    
    // Interactive mode
    printf("Type HELP for commands\n\n");
    
    char input[1024];
    while (1) {
        printf("Ok\n");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // Remove newline
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(input) == 0) continue;
        
        char *ptr = input;
        while (isspace(*ptr)) ptr++;
        
        // Check for commands
        if (strncmp(ptr, "QUIT", 4) == 0 || strncmp(ptr, "EXIT", 4) == 0) {
            break;
        } else if (strncmp(ptr, "HELP", 4) == 0) {
            print_help();
            continue;
        } else if (strncmp(ptr, "LIST", 4) == 0) {
            list_program(prog);
            continue;
        } else if (strncmp(ptr, "RUN", 3) == 0) {
            program_run(prog);
            continue;
        } else if (strncmp(ptr, "NEW", 3) == 0) {
            program_free(prog);
            prog = program_new();
            if (!prog) {
                fprintf(stderr, "Failed to create new program\n");
                return 1;
            }
            continue;
        }
        
        // Check if it's a numbered line
        if (isdigit(*ptr)) {
            int line_num = atoi(ptr);
            while (isdigit(*ptr)) ptr++;
            while (isspace(*ptr)) ptr++;
            program_add_line(prog, line_num, ptr);
        } else {
            // Direct execution (not implemented yet)
            printf("Direct execution not yet supported. Use line numbers.\n");
        }
    }
    
    program_free(prog);
    return 0;
}
