# INI Parser Library [![Awesome](https://awesome.re/badge.svg)](https://github.com/diffstorm/ini_parser)

[![Build Status](https://github.com/diffstorm/ini_parser/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/diffstorm/ini_parser/actions)
[![License](https://img.shields.io/github/license/diffstorm/ini_parser)](https://github.com/diffstorm/ini_parser/blob/main/LICENSE)
[![Language](https://img.shields.io/github/languages/top/diffstorm/ini_parser)](https://github.com/diffstorm/ini_parser)
[![Code Coverage](https://codecov.io/gh/diffstorm/ini_parser/branch/main/graph/badge.svg)](https://codecov.io/gh/diffstorm/ini_parser)
![GitHub Stars](https://img.shields.io/github/stars/diffstorm/ini_parser?style=social)
![Platforms](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)

A lightweight, single-header INI file parsing library written in C, with C++ compatibility. Focused on speed and safety, it offers a low-footprint, portable solution for decoding INI files. The library provides two APIs to suit different use cases: fast random access and memory-efficient streaming.

## Features
- **Dual Parsing Modes**
  - **Context API**: Builds searchable structure (O(1) lookups)
  - **Streaming API**: Zero-allocation event-based parsing
- **Memory Safe**: Automatic cleanup with context API
- **Unicode Ready**: Full UTF-8 support
- **Robust Parsing**: Handles sections, key-value pairs, comments, and empty lines
- **Memory Safe**: Automatic cleanup of allocated resources
- **Whitespace Handling**: Trims leading/trailing spaces in sections and values
- **Thread Safe**: Context-based design enables multi-instance usage
- **Cross-Platform**: C99/C++11 compatible with no external dependencies
- **Configurable**: Adjust line length (`INI_MAX_LINE_LENGTH`) and case sensitivity

## API Comparison

| Feature                | Context API (`ini_initialize`) | Streaming API (`ini_parse_stream`) |
|------------------------|--------------------------------|------------------------------------|
| **Memory Usage**       | Builds full structure          | Constant memory                    |
| **Lookup Speed**       | O(1) after init                | N/A (sequential)                   |
| **Best For**           | Small files, random access     | Large files (>1GB), embedded       |
| **Error Handling**     | Post-parse validation          | Immediate feedback                 |
| **Thread Safety**      | Per-context isolation          | Handler must be reentrant          |
| **Max File Size**      | Limited by memory              | Unlimited                          |
| **Needs init?**        | Yes (`ini_initialize`          | No                                 |
| **Needs cleanup?**     | Yes (`ini_cleanup`)            | No                                 |

**Why No Initialization/Cleanup Needed for Streaming:**
- Processes lines directly from input buffer
- Doesn't build any persistent data structures
- All temporary buffers are stack-allocated
- User handles state via `userdata` parameter

**Memory Diagram:**
```
Context API:              Streaming API:
+---------------+         +-----------------+
| ini_context_t |         | Input Buffer    |
| - sections    |         +-----------------+
| - keyValues   |         | Line Buffer     | (stack)
+---------------+         +-----------------+
| Allocations   |         | Userdata        |
+---------------+         +-----------------+
                          | No persistent   |
                          | state           |
                          +-----------------+
```

## Choosing an API

**Use Context API When:**
- You need random access to values
- Working with small to medium configs
- Frequent lookups needed after parsing

**Use Streaming API When:**
- Processing large files (>100MB)
- Memory-constrained environments
- Simple validation/transformation needed
- Direct loading to application structures

## Streaming Parsing API

### Core Components

```c
typedef enum {
    INI_EVENT_SECTION,    // New section found
    INI_EVENT_KEY_VALUE,  // Key-value pair parsed
    INI_EVENT_COMMENT,    // Comment line detected
    INI_EVENT_ERROR       // Parse error encountered
} ini_eventtype_t;

typedef bool (*ini_handler)(
    ini_eventtype_t type,  // Event type
    const char* section,   // Current section (if applicable)
    const char* key,       // Key name (for key-value events)
    const char* value,     // Value or comment content
    void* userdata         // Custom user context
);
```

### Functions

#### `bool ini_parse_stream(const char* content, size_t length, ini_handler handler, void* userdata)`
Processes INI content line-by-line
- `content`: Input string (not modified)
- `length`: Input length in bytes
- `handler`: Callback for parsed events
- `userdata`: Custom context pointer
- **Returns**: `false` if handler aborted parsing

### Usage Example

```c
typedef struct {
    int sections;
    int keys;
} ParserStats;

bool stats_handler(ini_eventtype_t type, 
                  const char* section,
                  const char* key,
                  const char* value,
                  void* userdata) {
    ParserStats* stats = (ParserStats*)userdata;
    
    switch(type) {
    case INI_EVENT_SECTION:
        stats->sections++;
        break;
    case INI_EVENT_KEY_VALUE:
        stats->keys++;
        printf("[%s] %s = %s\n", section, key, value);
        break;
    case INI_EVENT_ERROR:
        fprintf(stderr, "Error in: %s\n", value);
        return false; // Abort parsing
    }
    return true;
}

void process_large_ini(const char* content, size_t length) {
    ParserStats stats = {0};
    ini_parse_stream(content, length, stats_handler, &stats);
    printf("Parsed %d sections with %d keys\n", stats.sections, stats.keys);
}
```

## Context API

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

## Configuration Macros
- `INI_MAX_LINE_LENGTH`: Maximum allowed line length (default: 256)
- `INI_PARSER_IMPLEMENTATION`: Define to enable implementation inclusion
- `INI_ENABLE_CASE_SENSITIVITY`: Enables case sensitivity for sections, keys and values.

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

## Performance Tips

1. **Prefer Streaming API** for files >100MB
2. **Reuse Contexts** when possible
3. **Set INI_MAX_LINE_LENGTH** to match your data
4. **Avoid Case Sensitivity** unless required (`INI_ENABLE_CASE_SENSITIVITY`)
5. **Batch Initializations** for multiple small configs

## Quality Assurance

- **Valgrind Clean**: Zero memory leaks
- **100% Line Coverage**: Comprehensive test suite
- **Fuzz Tested**: AFL-integrated validation
- **SAN Enabled**: Address/UB sanitizer support

## :snowman: Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
