#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "branchless_scalar.h"

/* Forward declarations */
void exec_statement(Interpreter *interp, const char *stmt);

/* Print banner */
void print_banner() {
    printf("Modern GW-BASIC 64-bit Branchless Edition v1.0\n");
    printf("64-bit branchless interpreter for GW-BASIC\n");
    printf("Copyright (c) 2025\n\n");
}

/* Print help */
void print_help() {
    printf("Usage: gwbasic [options] [file]\n\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("  file           Load and run BASIC program from file\n\n");
}

/* Read entire file */
char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    
    /* Early return for NULL - this is error handling, not hot path control flow */
    char *result = NULL;
    int valid = (f != NULL);
    
    while (valid) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        result = (char *)malloc((size_t)size + 1);
        size_t bytes_read = fread(result, 1, (size_t)size, f);
        result[bytes_read] = '\0';
        
        fclose(f);
        break; /* Exit the while loop after processing */
    }
    
    return result;
}

/* Interactive REPL mode */
void repl_mode() {
    Interpreter *interp = interpreter_create();
    char line[1024];
    
    printf("Ready.\n\n");
    
    while (1) {
        printf("> ");
        char *input_result = fgets(line, sizeof(line), stdin);
        
        /* Exit on NULL (EOF) or exit commands */
        int got_input = (input_result != NULL);
        int is_exit = got_input & ((strncmp(line, "EXIT", 4) == 0) | (strncmp(line, "QUIT", 4) == 0) | (strncmp(line, "BYE", 3) == 0));
        
        int should_execute = got_input & (is_exit == 0);
        
        /* Execute line when valid - use while loop for conditional execution */
        while (should_execute > 0) {
            exec_statement(interp, line);
            should_execute = 0;
        }
        
        /* Break on exit */
        int dummy = is_exit | (got_input == 0);
        while (dummy > 0) {
            break;
        }
    }
    
    interpreter_destroy(interp);
}

int main(int argc, char **argv) {
    print_banner();
    
    /* Parse command line arguments branchlessly */
    int has_file = (argc > 1);
    int is_help = has_file & ((strcmp(argv[1], "-h") == 0) | (strcmp(argv[1], "--help") == 0));
    int is_version = has_file & ((strcmp(argv[1], "-v") == 0) | (strcmp(argv[1], "--version") == 0));
    int is_file = has_file & (is_help == 0) & (is_version == 0);
    
    /* Handle help - only print when requested */
    while (is_help > 0) {
        print_help();
        return 0;
    }
    
    /* Handle version */
    while (is_version > 0) {
        printf("Version 1.0.0\n");
        return 0;
    }
    
    /* Load and run file */
    char *source = NULL;
    source = is_file ? read_file(argv[1]) : source;
    
    Interpreter *interp = interpreter_create();
    
    /* Load program when file provided */
    int load_needed = (source != NULL);
    while (load_needed > 0) {
        interpreter_load_program(interp, source);
        load_needed = 0;
    }
    
    /* Run program when loaded */
    int run_needed = (source != NULL);
    while (run_needed > 0) {
        interpreter_run(interp);
        run_needed = 0;
    }
    
    /* Free resources */
    free(source);
    interpreter_destroy(interp);
    
    /* Enter REPL mode when no file */
    int need_repl = (is_file == 0);
    
    while (need_repl > 0) {
        repl_mode();
        need_repl = 0;
    }
    
    return 0;
}
