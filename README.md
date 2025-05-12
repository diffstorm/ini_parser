# INI Parser Library [![Awesome](https://awesome.re/badge.svg)](https://github.com/diffstorm/ini_parser)

[![Build Status](https://github.com/diffstorm/ini_parser/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/diffstorm/ini_parser/actions)
[![License](https://img.shields.io/github/license/diffstorm/ini_parser)](https://github.com/diffstorm/ini_parser/blob/main/LICENSE)
[![Language](https://img.shields.io/github/languages/top/diffstorm/ini_parser)](https://github.com/diffstorm/ini_parser)
[![Code Coverage](https://codecov.io/gh/diffstorm/ini_parser/branch/main/graph/badge.svg)](https://codecov.io/gh/diffstorm/ini_parser)
![GitHub Stars](https://img.shields.io/github/stars/diffstorm/ini_parser?style=social)
![Platforms](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)

A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

## Features

- **Robust Parsing**: Handles sections, key-value pairs, comments, and empty lines
- **Memory Safe**: Automatic cleanup of allocated resources
- **Whitespace Handling**: Trims leading/trailing spaces in sections and values
- **Thread Safe**: Context-based design enables multi-instance usage
- **Cross-Platform**: C99/C++11 compatible with no external dependencies
- **Configurable**: Adjustable line length via `INI_MAX_LINE_LENGTH`

## Components

### Core Structures
- **`ini_context_t`**: Manages parser state and stored sections
- **`ini_section_t`**: Represents an INI section with linked list of key-value pairs
- **`ini_keyvalue_t`**: Stores individual key-value entries

### Key Functions
- **Initialization**: `ini_initialize()` - Prepares parser context
- **Cleanup**: `ini_cleanup()` - Releases all allocated resources
- **Lookup**: `ini_hasSection()`, `ini_hasKey()`, `ini_getValue()`
- **Validation**: `ini_hasValue()` - Checks for non-empty values

## API Reference

### Data Structures

#### `ini_context_t`
Main parser context structure
- **Fields**:
  - `char *content`: Raw INI content (managed internally)
  - `ini_section_t *sections`: Linked list of parsed sections

#### `ini_section_t`
Represents an INI section
- **Fields**:
  - `char name[INI_MAX_LINE_LENGTH]`: Section name
  - `ini_keyvalue_t *keyValues`: Linked list of key-value pairs
  - `struct ini_section_t *next`: Pointer to next section

#### `ini_keyvalue_t`
Stores a key-value pair
- **Fields**:
  - `char key[INI_MAX_LINE_LENGTH]`: Entry key
  - `char value[INI_MAX_LINE_LENGTH]`: Entry value
  - `struct ini_keyvalue_t *next`: Pointer to next pair

### Functions

#### `bool ini_initialize(ini_context_t *ctx, const char *content, size_t length)`
Initializes parser context with INI content
- `ctx`: Context to initialize
- `content`: INI-formatted string
- `length`: Content length
- **Returns**: `true` on success, `false` on allocation failure

#### `void ini_cleanup(ini_context_t *ctx)`
Releases all resources associated with context
- Must be called after processing

#### `bool ini_getValue(const ini_context_t *ctx, const char *section, const char *key, char *value, size_t maxLen)`
Retrieves value for specified section/key
- `value`: Output buffer for value
- `maxLen`: Maximum buffer size
- **Returns**: `true` if value found and copied

### Configuration Macros
- `INI_MAX_LINE_LENGTH`: Maximum allowed line length (default: 256)
- `INI_PARSER_IMPLEMENTATION`: Define to enable implementation inclusion

## Error Handling
The parser provides implicit error checking through boolean return values. Common failure scenarios:
- Memory allocation failures during initialization
- Section/key not found in lookup functions
- Buffer overflow prevention in value retrieval

## Building

```bash
sudo apt-get install -y build-essential cmake libgtest-dev googletest
git clone https://github.com/diffstorm/ini_parser.git
cd ini_parser
mkdir build
cd build
cmake ..
cmake --build .
./c_demo
./cpp_demo
./ini_parser_tests
ctest --output-on-failure
```

## Usage Example

```c
#include "ini_parser.h"
#include <stdio.h>

int main() {
    const char *config = 
        "[network]\n"
        "host = 127.0.0.1\n"
        "port = 8080\n";
    
    ini_context_t ctx = {0};
    if (ini_initialize(&ctx, config, strlen(config))) {
        char host[256], port[16];
        if (ini_getValue(&ctx, "network", "host", host, sizeof(host)) &&
            ini_getValue(&ctx, "network", "port", port, sizeof(port))) {
            printf("Connecting to %s:%s\n", host, port);
        }
        ini_cleanup(&ctx);
    }
    return 0;
}
```

## Performance
- **O(1) Lookups**: Pre-parsed structure enables fast access
- **Low Memory**: Compact representation with single allocation
- **Efficient Parsing**: Linear time complexity O(n)

## :snowman: Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
