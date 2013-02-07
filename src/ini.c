/* inih -- simple .INI file parser

Forked by Fabio Pozzi <pozzi.fabio@gmail.com>
inih is released under the New BSD license (see LICENSE.txt). 
Go to the project
The original project home page for more info is:
http://code.google.com/p/inih/

*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "ini.h"

#define MAX_SECTION 50
#define MAX_NAME 50
#define RES_DIR "."

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

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file,
                   int (*handler)(void*, const char*, const char*,
                                  const char*),
                   void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
    char line[INI_MAX_LINE];
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through file line by line */
    while (fgets(line, INI_MAX_LINE, file) != NULL) {
        lineno++;

        start = line;
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-black line with leading whitespace, treat as continuation
               of previous name's value (as per Python ConfigParser). */
            if (!handler(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

/* See documentation in header file. */
int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

int replace_value(FILE *file, const char* filename, const char* val_section, const char *val_name, const char *new_value )
{
 
    char line[INI_MAX_LINE];
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;
    int ret,replace=0;
    FILE *newfile;
    char *new_filename, *tempfilename;
    /* Scan through file line by line */
    
    tempfilename= tempnam(RES_DIR, NULL);
    newfile = fopen(tempfilename, "w");
    if(!newfile){
            printf("temp filename doesn't exist");
            return -1;
    }
    new_filename = malloc(strlen(filename)+5);// ".bak" is 4 +\0 = 5
    while (fgets(line, INI_MAX_LINE, file) != NULL) {
        lineno++;

        start = line;
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
                ret = fprintf(newfile, "%s\n", line);
                if(ret == EOF){
                        printf("qualcosa non va\n");
                        return -1; // TODO: improve error handling
                }
        }
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                /*printf("linea:%s\n", line);*/
                ret = fprintf(newfile, "%s\n", line);
                if(ret == EOF)
                        return -1; // TODO: improve error handling
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
                /*  copy line inside new file */    
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);
                /* TODO: copiarti il commento da end+1 */

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                /*  TODO: refactor me */
                if(!strcmp(section, val_section)){
                        /*  we are in the right section */
                        if(!strcmp(name, val_name)){
                                /*also the name is right,
                                 * thus we can replace the line
                                 */
                               replace=1;
                        }
                        
                }
                if(replace){
                        memset(line, 0, INI_MAX_LINE); // not needed
                        snprintf(line, INI_MAX_LINE, "%s=%s", val_name, new_value);
                        replace=0;
                }
                else{
                        // I have to recreate the original row
                        snprintf(line, INI_MAX_LINE, "%s=%s", prev_name, value);
                }
                /*ret = printf("%s\n", line);*/
                ret = fprintf(newfile, "%s\n", line);
                if(ret == EOF)
                        return -1; // TODO: improve error handling
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }
    }
    fclose(file);
    /*  concatenate filename and new extension */
    new_filename[0]='\0';
    strcat(new_filename,filename);
    strcat(new_filename,".bak");
    remove(new_filename); //remove an existing file with the same name 
    rename(filename, new_filename); 
    fclose(newfile);
    ret = rename(tempfilename, filename);
    if(ret < 0){
            strerror(errno);
            return -1;
    }
    
    
    return error;
}

void print_help(void)
{
        printf("ini_demo: -f <filename> -s <section name> -n <key name> -v <new value> \n");
}

int main(int argc, char **argv)
{
    FILE* file;
    int error,ret;
    char *filename = NULL;
    char *section = NULL;
    char *name = NULL;
    char *value = NULL;

    if(argc < 4){
            printf("mancano dei parametri\n");
            print_help();
            return 0;
    }
    while((ret = getopt(argc, argv, "f:s:n:v:")) != -1){
            switch(ret)
            {
            case 'f':
                    filename = strdup(optarg);
                    break;
            case 's':
                    section = strdup(optarg);
                    break;
            case 'n':
                    name = strdup(optarg);
                    break;
            case 'v':
                    value = strdup(optarg);
                    break;
            default:
                    return -1;
            }
    }
    if(!filename || !section || !name || !value){
            printf("At least one argument is NULL");
            return 0;
    }
    file = fopen(filename, "r");
    if (!file){
        printf("file not found, exiting\n");
        return -1;
    }
    error = replace_value(file, filename, section, name, value);
    return error;
}
