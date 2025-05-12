/**
    @brief INI Parser Library

    A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

    @date 2025-05-12
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#define INI_PARSER_IMPLEMENTATION
#include "ini_parser.h"
#include <stdio.h>

int main(void)
{
    const char *iniContent =
        "\n"  // Empty line
        "[section1]\n"
        "  key1 = value1  \n"
        "key2=value2\n"
        ";\n"  // Empty comment
        "; Regular comment\n"
        "[section2]\n"
        "keyA=valueA\n"
        "emptyKey=\n";
    ini_context_t ctx = {0};

    if(!ini_initialize(&ctx, iniContent, strlen(iniContent)))
    {
        fprintf(stderr, "Initialization failed\n");
        return 1;
    }

    char value[256];
    printf("Section1 exists: %s\n", ini_hasSection(&ctx, "section1") ? "Yes" : "No");
    printf("Section3 exists: %s\n", ini_hasSection(&ctx, "section3") ? "Yes" : "No");

    if(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)))
    {
        printf("section1.key1 = '%s'\n", value);
    }

    printf("emptyKey has value: %s\n",
           ini_hasValue(&ctx, "section2", "emptyKey") ? "Yes" : "No");
    ini_cleanup(&ctx);
    return 0;
}