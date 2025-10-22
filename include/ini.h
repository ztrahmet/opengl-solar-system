/* inih -- simple .ini file parser
inih is released under the New BSD license (see LICENSE.txt). Go to
https://github.com/benhoyt/inih for more info.
*/

#ifndef __INI_H__
#define __INI_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Non-zero if ini_handler callback should accept null values */
#ifndef INI_ALLOW_NO_VALUE
#define INI_ALLOW_NO_VALUE 0
#endif

/* Non-zero if ini_handler callback should accept inline comments */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif

/* Non-zero if ini_handler callback should accept comments that start
   with a space */
#ifndef INI_ALLOW_SPACED_COMMENTS
#define INI_ALLOW_SPACED_COMMENTS 1
#endif

/* Non-zero if ini_handler callback should allow quotes in values */
#ifndef INI_ALLOW_QUOTES
#define INI_ALLOW_QUOTES 1
#endif

/* Non-zero if ini_handler callback should allow leading and trailing
   whitespace in section and name values */
#ifndef INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE
#define INI_ALLOW_LEADING_AND_TRAILING_WHITESPACE 1
#endif

/* Non-zero if ini_handler callback should stop parsing on first error */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 1
#endif

/* Non-zero if multi-line values should be allowed */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Non-zero if handler should be passed the line number */
#ifndef INI_HANDLER_LINENO
#define INI_HANDLER_LINENO 1
#endif

/* Non-zero if INI_CALL_HANDLER_ON_START_SECTION should be defined */
#ifndef INI_CALL_HANDLER_ON_START_SECTION
#define INI_CALL_HANDLER_ON_START_SECTION 0
#endif

/* Non-zero if ini_parse() should support BOM (Byte Order Mark) */
#ifndef INI_SUPPORT_BOM
#define INI_SUPPORT_BOM 1
#endif

/* Non-zero if ini_parse_stream() should convert newlines */
#ifndef INI_CONVERT_NEWLINES
#define INI_CONVERT_NEWLINES 1
#endif

/* Maximum line length for ini_parse_stream() */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

/* Buffer size for ini_parse_stream() */
#ifndef INI_INITIAL_BUFFER_SIZE
#define INI_INITIAL_BUFFER_SIZE 200
#endif

/* Default character to use for inline comments */
#ifndef INI_INLINE_COMMENT_CHAR
#define INI_INLINE_COMMENT_CHAR ';'
#endif

/* Default characters to use for comments */
#ifndef INI_START_COMMENT_CHARS
#define INI_START_COMMENT_CHARS ";#"
#endif

/* Non-zero to support name/value lines without an '=' sign */
#ifndef INI_ALLOW_NO_EQUAL
#define INI_ALLOW_NO_EQUAL 1
#endif

/* Non-zero to allow multi-line quoted values */
#ifndef INI_ALLOW_MULTILINE_QUOTES
#define INI_ALLOW_MULTILINE_QUOTES 1
#endif

/* Typedef for prototype of handler function */
#if INI_HANDLER_LINENO
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value,
                           int lineno);
#else
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value);
#endif

/* Typedef for prototype of fgets-style reader function */
typedef char* (*ini_reader)(char* str, int num, void* stream);


/* ini_parse(): Simple API that parses INI file from given filename
   (e.g. "config.ini").

   Returns 0 on success, line number of first error on parse error, or -1 on
   file open error. Optional INI_CALL_HANDLER_ON_START_SECTION flag calls
   handler on each section start (with name=NULL, value=NULL).
*/
int ini_parse(const char* filename, ini_handler handler, void* user);

/* ini_parse_file(): Simple API that parses INI file from given file pointer
   (e.g. stdin).

   Returns 0 on success, line number of first error on parse error. Optional
   INI_CALL_HANDLER_ON_START_SECTION flag calls handler on each section
   start (with name=NULL, value=NULL).
*/
int ini_parse_file(FILE* file, ini_handler handler, void* user);

/* ini_parse_stream(): High-level API that parses INI file from given
   ini_reader function.

   Returns 0 on success, line number of first error on parse error. Optional
   INI_CALL_HANDLER_ON_START_SECTION flag calls handler on each section
   start (with name=NULL, value=NULL).
*/
int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user);

/* ini_parse_string(): Simple API that parses INI file from given string.

   Returns 0 on success, line number of first error on parse error. Optional
   INI_CALL_HANDLER_ON_START_SECTION flag calls handler on each section
   start (with name=NULL, value=NULL).
*/
int ini_parse_string(const char* string, ini_handler handler, void* user);

/* Error codes returned by ini_parse() and ini_parse_file() */
#define INI_SUCCESS 0
#define INI_FILE_ERROR -1
#define INI_PARSE_ERROR -2

#ifdef __cplusplus
}
#endif

#endif /* __INI_H__ */
