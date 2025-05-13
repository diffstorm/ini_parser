/**
    @brief INI Parser Library

    A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

    @date 2025-05-12
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#ifndef INI_PARSER_H
#define INI_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>

#ifndef INI_MAX_LINE_LENGTH
#define INI_MAX_LINE_LENGTH 256
#endif

typedef enum
{
    INI_LINE_EMPTY,
    INI_LINE_SECTION,
    INI_LINE_KEY_VALUE,
    INI_LINE_COMMENT,
    INI_LINE_INVALID
} ini_linetype_t;

typedef struct ini_keyvalue_t
{
    char key[INI_MAX_LINE_LENGTH];
    char value[INI_MAX_LINE_LENGTH];
    struct ini_keyvalue_t *next;
} ini_keyvalue_t;

typedef struct ini_section_t
{
    char name[INI_MAX_LINE_LENGTH];
    ini_keyvalue_t *keyValues;
    struct ini_section_t *next;
} ini_section_t;

typedef struct
{
    char *content;
    ini_section_t *sections;
} ini_context_t;

bool ini_initialize(ini_context_t *ctx, const char *content, size_t length);
void ini_cleanup(ini_context_t *ctx);
bool ini_hasSection(const ini_context_t *ctx, const char *section);
bool ini_hasKey(const ini_context_t *ctx, const char *section, const char *key);
bool ini_hasValue(const ini_context_t *ctx, const char *section, const char *key);
bool ini_getValue(const ini_context_t *ctx, const char *section, const char *key,
                  char *value, size_t maxLen);

#ifdef __cplusplus
}
#endif

#endif /* INI_PARSER_H */

#ifdef INI_PARSER_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

static void trimWhitespace(char *str)
{
    if(!str || *str == '\0')
    {
        return;
    }

    char *start = str;

    while(isspace((unsigned char)*start))
    {
        start++;
    }

    char *end = str + strlen(str) - 1;

    while(end > start && isspace((unsigned char)*end))
    {
        end--;
    }

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

static ini_linetype_t parseLine(const char *line, char *section, char *key, char *value)
{
    while(isspace((unsigned char)*line))
    {
        line++;
    }

    if(*line == '\0')
    {
        return INI_LINE_EMPTY;
    }

    if(*line == ';' || *line == '#')
    {
        return INI_LINE_COMMENT;
    }

    if(*line == '[')
    {
        const char *start = ++line;

        while(*line && *line != ']')
        {
            line++;
        }

        if(*line == ']')
        {
            size_t len = line - start;
            len = (len < INI_MAX_LINE_LENGTH - 1) ? len : INI_MAX_LINE_LENGTH - 1;
            memcpy(section, start, len);
            section[len] = '\0';
            trimWhitespace(section);
            return INI_LINE_SECTION;
        }

        return INI_LINE_INVALID;
    }
    else
    {
        const char *keyStart = line;

        while(*line && *line != '=' && *line != ':')
        {
            line++;
        }

        if(*line == '\0')
        {
            return INI_LINE_INVALID;
        }

        size_t keyLen = line - keyStart;
        keyLen = (keyLen < INI_MAX_LINE_LENGTH - 1) ? keyLen : INI_MAX_LINE_LENGTH - 1;
        memcpy(key, keyStart, keyLen);
        key[keyLen] = '\0';
        trimWhitespace(key);
        line++;

        while(isspace((unsigned char)*line))
        {
            line++;
        }

        const char *valueStart = line;

        while(*line && *line != '\r' && *line != '\n')
        {
            line++;
        }

        size_t valueLen = line - valueStart;
        valueLen = (valueLen < INI_MAX_LINE_LENGTH - 1) ? valueLen : INI_MAX_LINE_LENGTH - 1;
        memcpy(value, valueStart, valueLen);
        value[valueLen] = '\0';
        trimWhitespace(value);
        return INI_LINE_KEY_VALUE;
    }

    return INI_LINE_INVALID;
}

bool ini_initialize(ini_context_t *ctx, const char *content, size_t length)
{
    if(!ctx || !content || length == 0)
    {
        return false;
    }

    ctx->content = malloc(length + 1);

    if(!ctx->content)
    {
        return false;
    }

    memcpy(ctx->content, content, length);
    ctx->content[length] = '\0';
    ini_section_t *currentSection = NULL;
    char line[INI_MAX_LINE_LENGTH];
    const char *ptr = ctx->content;

    while(*ptr)
    {
        const char *start = ptr;

        while(*ptr && *ptr != '\n')
        {
            ptr++;
        }

        size_t len = ptr - start;

        if(len > 0 && start[len - 1] == '\r')
        {
            len--;
        }

        len = (len < INI_MAX_LINE_LENGTH - 1) ? len : INI_MAX_LINE_LENGTH - 1;
        memcpy(line, start, len);
        line[len] = '\0';
        char section[INI_MAX_LINE_LENGTH] = {0};
        char key[INI_MAX_LINE_LENGTH] = {0};
        char value[INI_MAX_LINE_LENGTH] = {0};
        ini_linetype_t type = parseLine(line, section, key, value);

        if(type == INI_LINE_SECTION)
        {
            ini_section_t *newSection = malloc(sizeof(ini_section_t));

            if(!newSection)
            {
                ini_cleanup(ctx);
                return false;
            }

            strncpy(newSection->name, section, INI_MAX_LINE_LENGTH);
            newSection->keyValues = NULL;
            newSection->next = NULL;

            if(!ctx->sections)
            {
                ctx->sections = newSection;
            }
            else
            {
                ini_section_t *last = ctx->sections;

                while(last->next)
                {
                    last = last->next;
                }

                last->next = newSection;
            }

            currentSection = newSection;
        }
        else if(type == INI_LINE_KEY_VALUE && currentSection)
        {
            ini_keyvalue_t *newKv = malloc(sizeof(ini_keyvalue_t));

            if(!newKv)
            {
                ini_cleanup(ctx);
                return false;
            }

            strncpy(newKv->key, key, INI_MAX_LINE_LENGTH);
            strncpy(newKv->value, value, INI_MAX_LINE_LENGTH);
            newKv->next = NULL;

            if(!currentSection->keyValues)
            {
                currentSection->keyValues = newKv;
            }
            else
            {
                ini_keyvalue_t *last = currentSection->keyValues;

                while(last->next)
                {
                    last = last->next;
                }

                last->next = newKv;
            }
        }

        if(*ptr)
        {
            ptr++;
        }
    }

    return true;
}

void ini_cleanup(ini_context_t *ctx)
{
    if(!ctx)
    {
        return;
    }

    free(ctx->content);
    ini_section_t *section = ctx->sections;

    while(section)
    {
        ini_section_t *nextSection = section->next;
        ini_keyvalue_t *kv = section->keyValues;

        while(kv)
        {
            ini_keyvalue_t *nextKv = kv->next;
            free(kv);
            kv = nextKv;
        }

        free(section);
        section = nextSection;
    }

    ctx->sections = NULL;
}

bool ini_hasSection(const ini_context_t *ctx, const char *section)
{
    if(!ctx || !section)
    {
        return false;
    }

    ini_section_t *current = ctx->sections;

    while(current)
    {
        if(strcasecmp(current->name, section) == 0)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

bool ini_hasKey(const ini_context_t *ctx, const char *section, const char *key)
{
    if(!ctx || !section || !key)
    {
        return false;
    }

    ini_section_t *current = ctx->sections;

    while(current)
    {
        if(strcasecmp(current->name, section) == 0)
        {
            ini_keyvalue_t *kv = current->keyValues;

            while(kv)
            {
                if(strcasecmp(kv->key, key) == 0)
                {
                    return true;
                }

                kv = kv->next;
            }

            return false;
        }

        current = current->next;
    }

    return false;
}

bool ini_hasValue(const ini_context_t *ctx, const char *section, const char *key)
{
    char value[INI_MAX_LINE_LENGTH];
    return ini_getValue(ctx, section, key, value, sizeof(value)) && value[0] != '\0';
}

bool ini_getValue(const ini_context_t *ctx, const char *section, const char *key,
                  char *value, size_t maxLen)
{
    if(!ctx || !section || !key || !value || maxLen == 0)
    {
        return false;
    }

    ini_section_t *current = ctx->sections;
    bool found = false;

    while(current)
    {
        if(strcasecmp(current->name, section) == 0)
        {
            ini_keyvalue_t *kv = current->keyValues;

            while(kv)
            {
                if(strcasecmp(kv->key, key) == 0)
                {
                    strncpy(value, kv->value, maxLen);
                    value[maxLen - 1] = '\0';
                    found = true;
                }

                kv = kv->next;
            }

            return found;
        }

        current = current->next;
    }

    return false;
}

#endif /* INI_PARSER_IMPLEMENTATION */