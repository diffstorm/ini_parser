/**
    @brief INI Parser Library

    A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

    @date 2025-05-12
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#include "ini_parser.h"
#include <iostream>
#include <map>
#include <vector>
#include <cstring>

// Structure to collect parsing results
typedef struct
{
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::vector<std::string> comments;
    std::vector<std::string> errors;
    std::string current_section;
} ParserState;

// Handler function prototype
bool parsing_handler(ini_eventtype_t type,
                     const char *section,
                     const char *key,
                     const char *value,
                     void *userdata);

int main()
{
    const char *ini_content =
        "; Main configuration file\n"
        "[network]\n"
        "host = 127.0.0.1\n"
        "port = 8080\n"
        "[database]\n"
        "user = admin\n"
        "pass = secret\n"
        "[invalid_section\n"  // Missing closing bracket
        "key = value\n";
    ParserState state;
    state.current_section = INI_LINE_SECTION;
    size_t length = strlen(ini_content);
    bool success = ini_parse_stream(ini_content, length, parsing_handler, &state);
    // Display results
    std::cout << "Parsing " << (success ? "completed" : "aborted") << "\n\n";
    std::cout << "Comments (" << state.comments.size() << "):\n";

    for(const auto &comment : state.comments)
    {
        std::cout << "  " << comment << "\n";
    }

    std::cout << "\nErrors (" << state.errors.size() << "):\n";

    for(const auto &error : state.errors)
    {
        std::cout << "  " << error << "\n";
    }

    std::cout << "\nParsed data:\n";

    for(const auto& [section, values] : state.sections)
    {
        std::cout << "[" << section << "]\n";

        for(const auto& [key, val] : values)
        {
            std::cout << "  " << key << " = " << val << "\n";
        }
    }

    return 0;
}

bool parsing_handler(ini_eventtype_t type,
                     const char *section,
                     const char *key,
                     const char *value,
                     void *userdata)
{
    ParserState *state = static_cast<ParserState *>(userdata);

    switch(type)
    {
        case INI_EVENT_SECTION:
            state->current_section = section;
            state->sections[section]; // Create empty section
            break;

        case INI_EVENT_KEY_VALUE:
        {
            const char *actual_section = state->current_section.c_str();
            state->sections[actual_section][key] = value;
            break;
        }

        case INI_EVENT_COMMENT:
            state->comments.push_back(value);
            break;

        case INI_EVENT_ERROR:
            state->errors.push_back(value);
            return false; // Abort parsing on first error

        default:
            break;
    }

    return true;
}