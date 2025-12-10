#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        // Need to add new variable
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
    }
    
    // Free old string value if replacing
    if (var->type == VAR_STRING && var->value.string_val) {
        free(var->value.string_val);
        var->value.string_val = NULL;
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
