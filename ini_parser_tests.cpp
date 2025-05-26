/**
    @brief INI Parser Library

    A lightweight, single-header, speed and safety focused INI file parsing library written in C with C++ compatibility. Designed for simplicity and portability, this parser provides a low-footprint solution to decode INI format.

    @date 2025-05-12
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#include <gtest/gtest.h>
#include "ini_parser.h"
#include <string>
#include <cstring>

class IniParserTest : public ::testing::Test
{
protected:
    ini_context_t ctx;

    void SetUp() override
    {
        memset(&ctx, 0, sizeof(ini_context_t));
    }

    void TearDown() override
    {
        ini_cleanup(&ctx);
    }

    bool LoadIniContent(const char *content)
    {
        return ini_initialize(&ctx, content, strlen(content));
    }
};

TEST_F(IniParserTest, HandlesEmptyContent)
{
    const char *content = "";
    EXPECT_FALSE(ini_initialize(&ctx, content, strlen(content)));
}

TEST_F(IniParserTest, ParsesBasicStructure)
{
    const char *content =
        "[section1]\n"
        "key1=value1\n"
        "[section2]\n"
        "key2=value2\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasSection(&ctx, "section1"));
    EXPECT_TRUE(ini_hasSection(&ctx, "section2"));
    EXPECT_FALSE(ini_hasSection(&ctx, "section3"));
}

TEST_F(IniParserTest, HandlesWhitespace)
{
    const char *content =
        "[  section1  ]\r\n"
        "  key1 = value1  \n"
        "key2=  \n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasSection(&ctx, "section1"));
    char value[INI_MAX_LINE_LENGTH] = {0};
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1");
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "key2"));
    EXPECT_FALSE(ini_hasValue(&ctx, "section1", "key2"));
}

TEST_F(IniParserTest, HandlesCommentsAndEmptyLines)
{
    const char *content =
        "\n"
        "; Comment line\n"
        "# Another comment\n"
        "[section1]\n"
        "key1=value1 ; inline comment\n"
        "\n"
        "[section2]\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasSection(&ctx, "section1"));
    EXPECT_TRUE(ini_hasSection(&ctx, "section2"));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1 ; inline comment");
}

TEST_F(IniParserTest, CaseInsensitivity)
{
    const char *content =
        "[Section1]\n"
        "Key1=Value1\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasSection(&ctx, "sEcTiOn1"));
    EXPECT_TRUE(ini_hasKey(&ctx, "SECTION1", "kEy1"));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "KEY1", value, sizeof(value)));
    EXPECT_STREQ(value, "Value1");
}

TEST_F(IniParserTest, HandlesSpecialCharacters)
{
    const char *content =
        "[section!@#]\n"
        "key$%^=value&*()\n"
        "escaped_key=\"quoted value\"\n";
    ASSERT_TRUE(LoadIniContent(content));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_hasSection(&ctx, "section!@#"));
    EXPECT_TRUE(ini_hasKey(&ctx, "section!@#", "key$%^"));
    EXPECT_TRUE(ini_getValue(&ctx, "section!@#", "key$%^", value, sizeof(value)));
    EXPECT_STREQ(value, "value&*()");
    EXPECT_TRUE(ini_hasKey(&ctx, "section!@#", "escaped_key"));
    EXPECT_TRUE(ini_getValue(&ctx, "section!@#", "escaped_key", value, sizeof(value)));
    EXPECT_STREQ(value, "\"quoted value\"");
}

TEST_F(IniParserTest, HandlesDuplicateKeys)
{
    const char *content =
        "[section1]\n"
        "key1=first\n"
        "key1=second\n";
    ASSERT_TRUE(LoadIniContent(content));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "second");
}

TEST_F(IniParserTest, HandlesLongLines)
{
    constexpr size_t safe_len = INI_MAX_LINE_LENGTH / 2 - 2;
    std::string long_key(safe_len, 'a');
    std::string long_value(safe_len, 'b');
    std::string content =
        "[section1]\n" +
        long_key + "=" + long_value + "\n";
    ASSERT_TRUE(ini_initialize(&ctx, content.c_str(), content.length()));
    char value[INI_MAX_LINE_LENGTH] = {0};
    EXPECT_TRUE(ini_getValue(&ctx, "section1", long_key.c_str(), value, sizeof(value)));
    EXPECT_STREQ(value, long_value.c_str());
}

TEST_F(IniParserTest, BufferSafety)
{
    const char *content =
        "[section1]\n"
        "key1=value1\n";
    ASSERT_TRUE(LoadIniContent(content));
    char value[4];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "val");
}

TEST_F(IniParserTest, InvalidArguments)
{
    const char *content = "[section1]\nkey1=value1\n";
    ASSERT_TRUE(LoadIniContent(content));
    // Null context
    EXPECT_FALSE(ini_hasSection(nullptr, "section1"));
    // Null section/key
    EXPECT_FALSE(ini_hasSection(&ctx, nullptr));
    EXPECT_FALSE(ini_hasKey(&ctx, "section1", nullptr));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_FALSE(ini_getValue(&ctx, "section1", "key1", nullptr, sizeof(value)));
    EXPECT_FALSE(ini_getValue(&ctx, "section1", "key1", value, 0));
}

TEST_F(IniParserTest, MalformedLines)
{
    const char *content =
        "[section1\n"  // Missing closing bracket
        "key1\n"       // Missing value
        "=value1\n"    // Missing key
        "key2:value2\n"// Colon separator
        "[section2]\n"
        "key3=value3\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasSection(&ctx, "section2"));
    EXPECT_FALSE(ini_hasSection(&ctx, "section1"));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section2", "key3", value, sizeof(value)));
    EXPECT_STREQ(value, "value3");
}

TEST_F(IniParserTest, EmptyValues)
{
    const char *content =
        "[section1]\n"
        "empty1=\n"
        "empty2=  \n"
        "valid=value\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "empty1"));
    EXPECT_FALSE(ini_hasValue(&ctx, "section1", "empty1"));
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "empty2"));
    EXPECT_FALSE(ini_hasValue(&ctx, "section1", "empty2"));
    EXPECT_TRUE(ini_hasValue(&ctx, "section1", "valid"));
}

TEST_F(IniParserTest, MultiSectionOperations)
{
    const char *content =
        "[sectionA]\n"
        "key1=value1\n"
        "[sectionB]\n"
        "key1=value2\n";
    ASSERT_TRUE(LoadIniContent(content));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "sectionA", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1");
    EXPECT_TRUE(ini_getValue(&ctx, "sectionB", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value2");
}

TEST_F(IniParserTest, KeyWithoutSection)
{
    const char *content =
        "key1=value1\n"
        "[section1]\n"
        "key2=value2\n";
    ASSERT_TRUE(LoadIniContent(content));
    EXPECT_FALSE(ini_hasKey(&ctx, "", "key1"));
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "key2"));
}

TEST_F(IniParserTest, HandlesCRLFFiles)
{
    const char *content = "[section]\r\nkey=value\r\n";
    ASSERT_TRUE(LoadIniContent(content));
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section", "key", value, sizeof(value)));
    EXPECT_STREQ(value, "value");
}

TEST_F(IniParserTest, MaxLineLengthHandling)
{
    std::string long_line(INI_MAX_LINE_LENGTH * 2, 'a');
    std::string content = "[section]\n" + long_line + "\n";
    ASSERT_TRUE(ini_initialize(&ctx, content.c_str(), content.size()));
}

TEST_F(IniParserTest, MemorySafetyOnInvalidInput)
{
    const char *content = "[section\nkey=value"; // Missing ] and unterminated quote
    ini_context_t ctx;
    EXPECT_FALSE(ini_initialize(&ctx, content, strlen(content)));
    // Safe to call multiple times
    ini_cleanup(&ctx);
    ini_cleanup(&ctx); // Double call test
    ini_cleanup(&ctx); // Triple call test
}

TEST_F(IniParserTest, ComprehensiveParsing)
{
    const char *content = R"INI(
[Basic_Types]
string = Hello World
integer = 42
float = 3.14159
true = true
false = false
empty = 

[Whitespace]
  spaced_key   =   value with spaces  
	tabs_key	=	value	with	tabs	
mixed_whitespace 	 =  	 complex   example   

[Duplicate_Keys]
duplicate = first
duplicate = second

[Special_Characters]
special_chars = "!@#$%^&*()_+-=[]{}|;':\",./<>?"
quoted = "quoted value"

[Empty_Section]

[Comments]
# Hash comment
; Semicolon comment
key = value

[Order_Verification]
key1 = 1
key2 = 2
key3 = 3
)INI";

    ASSERT_TRUE(LoadIniContent(content));

    // Basic Types Verification
    char buffer[INI_MAX_LINE_LENGTH];
    
    // String
    EXPECT_TRUE(ini_getValue(&ctx, "Basic_Types", "string", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "Hello World");
    
    // Integer
    EXPECT_TRUE(ini_getValue(&ctx, "Basic_Types", "integer", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "42");
    
    // Float
    EXPECT_TRUE(ini_getValue(&ctx, "Basic_Types", "float", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "3.14159");
    
    // Booleans
    EXPECT_TRUE(ini_getValue(&ctx, "Basic_Types", "true", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "true");
    EXPECT_TRUE(ini_getValue(&ctx, "Basic_Types", "false", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "false");
    
    // Empty value
    EXPECT_TRUE(ini_hasKey(&ctx, "Basic_Types", "empty"));
    EXPECT_FALSE(ini_hasValue(&ctx, "Basic_Types", "empty"));

    // Whitespace Handling
    EXPECT_TRUE(ini_getValue(&ctx, "Whitespace", "spaced_key", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "value with spaces");
    
    EXPECT_TRUE(ini_getValue(&ctx, "Whitespace", "tabs_key", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "value	with	tabs");
    
    EXPECT_TRUE(ini_getValue(&ctx, "Whitespace", "mixed_whitespace", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "complex   example");

    // Duplicate Keys
    EXPECT_TRUE(ini_getValue(&ctx, "Duplicate_Keys", "duplicate", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "second");

    // Special Characters
    EXPECT_TRUE(ini_getValue(&ctx, "Special_Characters", "special_chars", buffer, sizeof(buffer)));
	EXPECT_STREQ(buffer, "\"!@#$%^&*()_+-=[]{}|;':\\\",./<>?\"");
    
    EXPECT_TRUE(ini_getValue(&ctx, "Special_Characters", "quoted", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "\"quoted value\"");

    // Empty Section
    EXPECT_TRUE(ini_hasSection(&ctx, "Empty_Section"));

    // Comments
    EXPECT_TRUE(ini_getValue(&ctx, "Comments", "key", buffer, sizeof(buffer)));
    EXPECT_STREQ(buffer, "value");

    // Order Preservation
    ini_section_t* section = ctx.sections;
    while(section && STRCOMPARE(section->name, "Order_Verification") != 0) {
        section = section->next;
    }
    ASSERT_NE(section, nullptr) << "Order_Verification section not found";
    
    ini_keyvalue_t* kv = section->keyValues;
    ASSERT_NE(kv, nullptr) << "No keys in Order_Verification";
    EXPECT_STREQ(kv->key, "key1");
    EXPECT_STREQ(kv->value, "1");
    
    kv = kv->next;
    ASSERT_NE(kv, nullptr) << "Missing key2";
    EXPECT_STREQ(kv->key, "key2");
    EXPECT_STREQ(kv->value, "2");
    
    kv = kv->next;
    ASSERT_NE(kv, nullptr) << "Missing key3";
    EXPECT_STREQ(kv->key, "key3");
    EXPECT_STREQ(kv->value, "3");
    
    EXPECT_EQ(kv->next, nullptr) << "Extra unexpected keys in section";
}

TEST_F(IniParserTest, StreamingAPIParsing) {
    const char* content = R"INI(
; Test config
[Section1]
key1 = value1
key2 = value2

[Section2]
# Invalid line
key3 = value3
invalid_line
key4 = value4
)INI";

    typedef struct {
        std::vector<std::string> sections;
        std::map<std::string, std::string> keyValues;
        std::vector<std::string> comments;
        std::vector<std::string> errors;
        bool aborted;
    } StreamTestState;

    StreamTestState state{};
    state.aborted = false;

    auto handler = [](ini_eventtype_t type, 
                     const char* section,
                     const char* key,
                     const char* value,
                     void* userdata) -> bool {
        StreamTestState* s = static_cast<StreamTestState*>(userdata);
        
        switch(type) {
            case INI_EVENT_SECTION:
                s->sections.push_back(section ? section : "");
                break;
                
            case INI_EVENT_KEY_VALUE:
                if(section && key && value) {
                    std::string fullKey = std::string(section) + "." + key;
                    s->keyValues[fullKey] = value;
                }
                break;
                
            case INI_EVENT_COMMENT:
                if(value) s->comments.push_back(value);
                break;
                
            case INI_EVENT_ERROR:
                if(value) s->errors.push_back(value);
                s->aborted = true;
                return false;
                
            default: break;
        }
        return true;
    };

    size_t length = strlen(content);
    bool result = ini_parse_stream(content, length, handler, &state);

    EXPECT_FALSE(result);
    EXPECT_TRUE(state.aborted);
    ASSERT_EQ(state.sections.size(), 2);
    ASSERT_EQ(state.keyValues.size(), 3);
    ASSERT_EQ(state.comments.size(), 2);
    ASSERT_EQ(state.errors.size(), 1);
}

TEST_F(IniParserTest, StreamingAPIEdgeCases) {
    // Test empty content
    {
        bool called = false;
        auto handler = [](ini_eventtype_t, const char*, const char*, const char*, void* userdata) {
            if(userdata) *static_cast<bool*>(userdata) = true;
            return true;
        };
        EXPECT_TRUE(ini_parse_stream("", 0, handler, &called));
        EXPECT_FALSE(called);
    }

    // Test comment-only file
    {
        struct CommentCounter { int count = 0; };
        CommentCounter counter;
        
        auto handler = [](ini_eventtype_t type, const char*, const char*, const char* value, void* userdata) {
            if(type == INI_EVENT_COMMENT && userdata) {
                static_cast<CommentCounter*>(userdata)->count++;
            }
            return true;
        };
        
        const char* content = "; comment1\n# comment2\n";
        EXPECT_TRUE(ini_parse_stream(content, strlen(content), handler, &counter));
        EXPECT_EQ(counter.count, 2);
    }

    // Test handler abort
    {
        struct Counter { int value = 0; };
        Counter counter;
        
        auto handler = [](ini_eventtype_t, const char*, const char*, const char*, void* userdata) {
            if(userdata) static_cast<Counter*>(userdata)->value++;
            return static_cast<Counter*>(userdata)->value < 2;
        };
        
        const char* content = "[s1]\nkey1=1\nkey2=2\n";
        EXPECT_FALSE(ini_parse_stream(content, strlen(content), handler, &counter));
        EXPECT_EQ(counter.value, 2);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}