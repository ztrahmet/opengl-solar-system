/* inih -- simple .ini file parser
inih is released under the New BSD license (see LICENSE.txt). Go to
https://github.com/benhoyt/inih for more info.
*/

#include "ini.h"

#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#elif defined(_MSC_VER)
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !INI_CUSTOM_ALLOCATOR
#include <stdlib.h>
#define INI_MALLOC(sz) malloc(sz)
#define INI_FREE(p) free(p)
#define INI_REALLOC(p, sz) realloc(p, sz)
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Used by ini_parse_string() to keep track of string information */
typedef struct {
    const char* ptr;
    size_t num_left;
} ini_parse_string_ctx;

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char c or a newline in given string, or pointer to
   null at end of string if neither found. Inline comments are also handled. */
static char* find_char_or_comment(const char* s, char c)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && *s != c && !(was_space && *s == INI_INLINE_COMMENT_CHAR)) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && *s != c) {
        s++;
    }
#endif
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    size_t len = strnlen(src, size - 1);
    memcpy(dest, src, len);
    dest[len] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user)
{
    /* Used to make sure we don't clobber the INI_INITIAL_BUFFER_SIZE buffer */
    size_t buffer_size = INI_INITIAL_BUFFER_SIZE;
    char* buffer = (char*)INI_MALLOC(INI_INITIAL_BUFFER_SIZE);
    if (!buffer) {
        return INI_PARSE_ERROR;
    }

    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* line;
    int lineno = 0;
    int error = 0;

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v, l) handler(u, s, n, v, l)
#else
#define HANDLER(u, s, n, v, l) handler(u, s, n, v)
#endif

    /* Scan through stream line by line */
    while (reader((line = buffer), (int)buffer_size, stream)) {
#if INI_CONVERT_NEWLINES
        /* Ensure that the buffer is resized if the line is too long */
        size_t len = strlen(line);
        while (len == buffer_size - 1 && line[len - 1] != '\n') {
            size_t new_size = buffer_size * 2;
            if (new_size < buffer_size) {
                /* Overflow */
                INI_FREE(buffer);
                return INI_PARSE_ERROR;
            }
            char* new_buffer = (char*)INI_REALLOC(buffer, new_size);
            if (!new_buffer) {
                INI_FREE(buffer);
                return INI_PARSE_ERROR;
            }
            buffer = new_buffer;
            buffer_size = new_size;
            if (!reader(buffer + len, (int)(buffer_size - len), stream))
                break;
            len = strlen(buffer);
        }

        /* Convert newlines to '\0' */
        char* p = line;
        while ((p = strpbrk(p, "\r\n")))
            *p = '\0';
#endif

        lineno++;
        error = 0;
        char* start = line;
#if INI_SUPPORT_BOM
        /* Support BOM (Byte Order Mark) */
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                            (unsigned char)start[1] == 0xBB &&
                            (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (!*start) {
            /* Blank line */
            continue;
        }
#if INI_ALLOW_SPACED_COMMENTS
        else if (*start == INI_START_COMMENT_CHARS[0] || *start == INI_START_COMMENT_CHARS[1]) {
#else
        else if (strchr(INI_START_COMMENT_CHARS, *start)) {
#endif
            /* Comment line */
            continue;
        }
#if INI_CALL_HANDLER_ON_START_SECTION
        else if (*start == '[') {
#else
        else if (*start == '[') {
#endif
            /* A "[section]" line */
            char* end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
#if INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE
                rstrip(section);
                char* section_start = lskip(section);
                if (section_start != section) {
                    strncpy0(section, section_start, sizeof(section));
                }
#endif
                *prev_name = '\0';
#if INI_CALL_HANDLER_ON_START_SECTION
                if (!HANDLER(user, section, NULL, NULL, lineno)) {
                    error = 1;
                }
#endif
            }
            else {
                /* No ']' found on section line */
                error = 1;
            }
        }
        else {
            /* A "name=value" line */
            char* equal = find_char_or_comment(start, '=');
            if (*equal == '=') {
                *equal = '\0';
                char* name = rstrip(start);
                char* value = equal + 1;
#if INI_ALLOW_INLINE_COMMENTS
                char* comment = find_char_or_comment(value, '\0');
                if (*comment == INI_INLINE_COMMENT_CHAR)
                    *comment = '\0';
#endif
#if INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE
                value = lskip(value);
#endif
                rstrip(value);

#if INI_ALLOW_QUOTES
                /* Handle quoted values */
                if (strlen(value) >= 2 && *value == '"' && value[strlen(value) - 1] == '"') {
                    value++;
                    value[strlen(value) - 1] = '\0';
#if INI_ALLOW_MULTILINE_QUOTES == 0
                    /* Check for internal newlines */
                    if (strchr(value, '\n') || strchr(value, '\r')) {
                        error = 1;
                    }
#endif
                }
#endif /* INI_ALLOW_QUOTES */
#if INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE
                name = lskip(name);
#endif

                strncpy0(prev_name, name, sizeof(prev_name));
                if (!HANDLER(user, section, name, value, lineno)) {
                    error = 1;
                }
            }
#if INI_ALLOW_NO_EQUAL
            else {
                /* A "name" line with no value */
#if INI_ALLOW_NO_VALUE
                char* name = rstrip(start);
#if INI_ALLOW_INLINE_COMMENTS
                char* comment = find_char_or_comment(name, '\0');
                if (*comment == INI_INLINE_COMMENT_CHAR)
                    *comment = '\0';
#endif
#if INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE
                name = lskip(name);
#endif
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!HANDLER(user, section, name, NULL, lineno)) {
                    error = 1;
                }
#else /* INI_ALLOW_NO_VALUE */
                error = 1;
#endif /* INI_ALLOW_NO_VALUE */
            }
#endif /* INI_ALLOW_NO_EQUAL */
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error) {
            INI_FREE(buffer);
            return lineno;
        }
#endif
    }

    INI_FREE(buffer);
    return INI_SUCCESS;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return INI_FILE_ERROR;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

/* Internal reader function for ini_parse_string() */
static char* ini_parse_string_reader(char* str, int num, void* stream) {
    ini_parse_string_ctx* ctx = (ini_parse_string_ctx*)stream;
    const char* ptr = ctx->ptr;
    size_t len = 0;

    if (!ctx->num_left)
        return NULL;

    while (len < (size_t)num - 1 && len < ctx->num_left && *ptr != '\n') {
        str[len++] = *ptr++;
    }
    if (len < ctx->num_left && *ptr == '\n') {
        str[len++] = *ptr++;
    }

    str[len] = '\0';
    ctx->num_left -= len;
    ctx->ptr = ptr;
    return str;
}

/* See documentation in header file. */
int ini_parse_string(const char* string, ini_handler handler, void* user) {
    ini_parse_string_ctx ctx = { string, strlen(string) };
    return ini_parse_stream(ini_parse_string_reader, &ctx, handler, user);
}
