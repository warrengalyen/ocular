#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PARAMS 10
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 50

typedef enum {
    PARAM_FLOAT,
    PARAM_BOOL
} ParamType;

typedef struct {
    char key[MAX_KEY_LENGTH];
    ParamType type;
    union {
        float float_val;
        bool bool_val;
    } value;
} Param;

typedef struct {
    char function[MAX_KEY_LENGTH];
    bool use_stride;
    Param params[MAX_PARAMS];
    int param_count;
} Config;

char* stripComments(char* line) {
    char* comment = strchr(line, '#');
    if (comment) {
        *comment = '\0';  // Terminate string at comment
    }
    return line;
}

bool isBoolean(const char* str) {
    return (strcmp(str, "true") == 0 || 
            strcmp(str, "false") == 0 || 
            strcmp(str, "yes") == 0 || 
            strcmp(str, "no") == 0);
}

bool parseBoolean(const char* str) {
    return (strcmp(str, "true") == 0 || strcmp(str, "yes") == 0);
}

void parseConfigFile(const char* filename, Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open config file: %s\n", filename);
        return;
    }

    char line[256];
    config->param_count = 0;
    int indent_level = 0;
    bool in_params_section = false;

    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines
        if (line[0] == '\n') {
            continue;
        }

        // Strip comments before processing
        stripComments(line);

        // Remove trailing whitespace
        char* end = line + strlen(line) - 1;
        while (end > line && (*end == '\n' || *end == ' ' || *end == '\r')) {
            *end-- = '\0';
        }

        // Skip if line is empty after removing comments
        if (strlen(line) == 0) {
            continue;
        }

        // Count leading spaces for indent level
        char* ptr = line;
        indent_level = 0;
        while (*ptr == ' ') {
            indent_level++;
            ptr++;
        }

        // Process YAML-style lines
        if (strstr(ptr, "function:") == ptr) {
            sscanf(ptr + 9, " %s", config->function);
        }
        else if (strstr(ptr, "use_stride:") == ptr) {
            char value[10];
            sscanf(ptr + 11, " %s", value);
            config->use_stride = parseBoolean(value);
        }
        else if (strstr(ptr, "params:") == ptr) {
            in_params_section = true;
            continue;
        }
        else if (in_params_section && strchr(ptr, ':') != NULL) {
            // Handle indented parameter entries
            if (indent_level >= 2) {
                if (config->param_count >= MAX_PARAMS) {
                    fprintf(stderr, "Warning: Maximum number of parameters (%d) exceeded\n", MAX_PARAMS);
                    continue;
                }
                char key[MAX_KEY_LENGTH];
                char value_str[MAX_VALUE_LENGTH];
                
                if (sscanf(ptr, " %[^:]: %s", key, value_str) == 2) {
                    strcpy(config->params[config->param_count].key, key);
                    
                    // Check if value is boolean
                    if (isBoolean(value_str)) {
                        config->params[config->param_count].type = PARAM_BOOL;
                        config->params[config->param_count].value.bool_val = parseBoolean(value_str);
                    } else {
                        // Try to parse as float
                        config->params[config->param_count].type = PARAM_FLOAT;
                        config->params[config->param_count].value.float_val = atof(value_str);
                    }
                    
                    config->param_count++;
                }
            }
        }
    }

    fclose(file);
}

#endif
