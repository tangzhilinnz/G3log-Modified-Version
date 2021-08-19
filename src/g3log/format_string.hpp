#pragma once

#include <string.h>
#include <stdarg.h> // va_list and its operations

int
tz_vsnprintf(
    char* buffer,         // buffer to write to
    size_t count,	      // max number of characters to store in buffer
    const char* fmt,      // format string
    va_list	vaList        // optional arguments to format
);

int 
tz_snprintf(
    char* buffer,         // buffer to write to
    size_t count,         // max number of characters to store in buffer
    const char* fmt,      // format string
    ...                   // optional arguments to format
);

