#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "../include/gwbasic.h"

#define INITIAL_CAPACITY 16

SymbolTable *symbol_table_new(void) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table) return NULL;
    
    table->vars = malloc(INITIAL_CAPACITY * sizeof(Variable));
    if (!table->vars) {
        free(table);
        return NULL;
    }
    
    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    return table;
}

void symbol_table_free(SymbolTable *table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->count; i++) {
        if (table->vars[i].type == VAR_STRING && table->vars[i].value.string_val) {
            free(table->vars[i].value.string_val);
        }
        if (table->vars[i].is_array && table->vars[i].array_data) {
            free(table->vars[i].array_data);
        }
    }
    
    free(table->vars);
    free(table);
}

Variable *symbol_table_get(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->vars[i].name, name) == 0) {
            return &table->vars[i];
        }
    }
    
    return NULL;
}

void symbol_table_set(SymbolTable *table, const char *name, VarType type, void *value) {
    if (!table || !name) return;
    
    Variable *var = symbol_table_get(table, name);
    
    if (!var) {
        /* Need to add new variable */
        if (table->count >= table->capacity) {
            size_t new_capacity = table->capacity * 2;
            Variable *new_vars = realloc(table->vars, new_capacity * sizeof(Variable));
            if (!new_vars) return;
            table->vars = new_vars;
            table->capacity = new_capacity;
        }
        
        var = &table->vars[table->count++];
        strncpy(var->name, name, sizeof(var->name) - 1);
        var->name[sizeof(var->name) - 1] = '\0';
        var->type = type;
        var->is_array = false;
        var->array_data = NULL;
        var->dim_count = 0;
    }
    
    /* Free old string value if the variable was previously a string */
    VarType old_type = var->type;
    if (old_type == VAR_STRING && var->value.string_val) {
        free(var->value.string_val);
        var->value.string_val = NULL;
    }
    
    /* Free old array data if the variable was previously an array */
    if (var->is_array && var->array_data) {
        free(var->array_data);
        var->array_data = NULL;
        var->is_array = false;
        var->dim_count = 0;
    }
    
    var->type = type;
    
    switch (type) {
        case VAR_INTEGER:
            var->value.int_val = *(int64_t *)value;
            break;
        case VAR_SINGLE:
            var->value.float_val = *(float *)value;
            break;
        case VAR_DOUBLE:
            var->value.double_val = *(double *)value;
            break;
        case VAR_STRING:
            var->value.string_val = strdup((char *)value);
            if (!var->value.string_val) {
                fprintf(stderr, "Warning: Failed to allocate string memory\n");
                var->value.string_val = NULL;
            }
            break;
    }
}

void symbol_table_set_array(SymbolTable *table, const char *name, int *dims, int dim_count) {
    if (!table || !name || !dims || dim_count <= 0) return;
    
    Variable *var = symbol_table_get(table, name);
    
    if (!var) {
        if (table->count >= table->capacity) {
            size_t new_capacity = table->capacity * 2;
            Variable *new_vars = realloc(table->vars, new_capacity * sizeof(Variable));
            if (!new_vars) return;
            table->vars = new_vars;
            table->capacity = new_capacity;
        }
        
        var = &table->vars[table->count++];
        strncpy(var->name, name, sizeof(var->name) - 1);
        var->name[sizeof(var->name) - 1] = '\0';
    }
    
    /* Calculate total size with overflow check */
    size_t total_size = 1;
    for (int i = 0; i < dim_count && i < 8; i++) {
        var->dims[i] = dims[i];
        /* Check for overflow before multiplication */
        if (total_size > SIZE_MAX / dims[i]) {
            fprintf(stderr, "Error: Array dimensions too large\n");
            return;
        }
        total_size *= dims[i];
    }
    var->dim_count = dim_count;
    
    /* Allocate array storage (default to doubles) */
    if (var->array_data) free(var->array_data);
    var->array_data = calloc(total_size, sizeof(double));
    
    /* Check allocation success */
    if (!var->array_data) {
        fprintf(stderr, "Error: Failed to allocate array memory\n");
        var->is_array = false;
        var->dim_count = 0;
        return;
    }
    
    var->is_array = true;
    var->type = VAR_DOUBLE;
}

