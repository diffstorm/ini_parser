/**
    @brief INI Parser Library

	A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

    @date 2025-05-12
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#include <iostream>
#include <cstring>
#include "ini_parser.h"

int main() {
    const char* iniContent = 
        "\n"
        "[section1]\n"
        "  key1 = value1  \n"
        "key2=value2\n"
        ";\n"
        "; Regular comment\n"
        "[section2]\n"
        "keyA=valueA\n"
        "emptyKey=\n";

    ini_context_t ctx{};
    if (!ini_initialize(&ctx, iniContent, std::strlen(iniContent))) {
        std::cerr << "Initialization failed\n";
        return 1;
    }

    char value[256];
    std::cout << "Section1 exists: " 
              << (ini_hasSection(&ctx, "section1") ? "Yes" : "No") << "\n";
    std::cout << "Section3 exists: " 
              << (ini_hasSection(&ctx, "section3") ? "Yes" : "No") << "\n";

    if (ini_getValue(&ctx, "section1", "key1", value, sizeof(value))) {
        std::cout << "section1.key1 = '" << value << "'\n";
    }

    std::cout << "emptyKey has value: " 
              << (ini_hasValue(&ctx, "section2", "emptyKey") ? "Yes" : "No") << "\n";

    ini_cleanup(&ctx);
    return 0;
}