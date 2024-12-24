#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

bool is_valid_string(const char* str) {
    
	if (str == NULL) {
        return false;
    }

    while (*str != '\0') {
        if (!(isalnum(*str) || *str == '_' || *str == '-')) {
            return false;
        }
        str++;
    }
    return true;
}
