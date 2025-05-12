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

class IniParserTest : public ::testing::Test {
protected:
    ini_context_t ctx;

    void SetUp() override {
        memset(&ctx, 0, sizeof(ini_context_t));
    }

    void TearDown() override {
        ini_cleanup(&ctx);
    }

    void LoadIniContent(const char* content) {
        ASSERT_TRUE(ini_initialize(&ctx, content, strlen(content)));
    }
};

TEST_F(IniParserTest, HandlesEmptyContent) {
    const char* content = "";
    EXPECT_FALSE(ini_initialize(&ctx, content, strlen(content)));
}

TEST_F(IniParserTest, ParsesBasicStructure) {
    const char* content = 
        "[section1]\n"
        "key1=value1\n"
        "[section2]\n"
        "key2=value2\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    EXPECT_TRUE(ini_hasSection(&ctx, "section1"));
    EXPECT_TRUE(ini_hasSection(&ctx, "section2"));
    EXPECT_FALSE(ini_hasSection(&ctx, "section3"));
}

TEST_F(IniParserTest, HandlesWhitespace) {
    const char* content = 
        "  [  section1  ]  \n"
        "  key1  =  value1  \n"
        "key2=  \n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1");
    
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "key2"));
    EXPECT_FALSE(ini_hasValue(&ctx, "section1", "key2"));
}

TEST_F(IniParserTest, HandlesCommentsAndEmptyLines) {
    const char* content = 
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
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1 ; inline comment");
}

TEST_F(IniParserTest, CaseInsensitivity) {
    const char* content = 
        "[Section1]\n"
        "Key1=Value1\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    EXPECT_TRUE(ini_hasSection(&ctx, "sEcTiOn1"));
    EXPECT_TRUE(ini_hasKey(&ctx, "SECTION1", "kEy1"));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "KEY1", value, sizeof(value)));
    EXPECT_STREQ(value, "Value1");
}

TEST_F(IniParserTest, HandlesSpecialCharacters) {
    const char* content = 
        "[section!@#]\n"
        "key$%^=value&*()\n"
        "escaped_key=\"quoted value\"\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section!@#", "key$%^", value, sizeof(value)));
    EXPECT_STREQ(value, "value&*()");
    
    EXPECT_TRUE(ini_getValue(&ctx, "section!@#", "escaped_key", value, sizeof(value)));
    EXPECT_STREQ(value, "\"quoted value\"");
}

TEST_F(IniParserTest, HandlesDuplicateKeys) {
    const char* content = 
        "[section1]\n"
        "key1=first\n"
        "key1=second\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "second");
}

TEST_F(IniParserTest, HandlesLongLines) {
    std::string long_key(INI_MAX_LINE_LENGTH - 10, 'a');
    std::string long_value(INI_MAX_LINE_LENGTH - 10, 'b');
    
    std::string content = 
        "[section1]\n" +
        long_key + "=" + long_value + "\n";
    
    ASSERT_TRUE(ini_initialize(&ctx, content.c_str(), content.length()));
    
    char value[INI_MAX_LINE_LENGTH];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", long_key.c_str(), value, sizeof(value)));
    EXPECT_STREQ(value, long_value.c_str());
}

TEST_F(IniParserTest, BufferSafety) {
    const char* content = 
        "[section1]\n"
        "key1=value1\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    char value[4];
    EXPECT_TRUE(ini_getValue(&ctx, "section1", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "val");
}

TEST_F(IniParserTest, InvalidArguments) {
    const char* content = "[section1]\nkey1=value1\n";
    ASSERT_TRUE(LoadIniContent(content));
    
    // Null context
    EXPECT_FALSE(ini_hasSection(nullptr, "section1"));
    
    // Null section/key
    EXPECT_FALSE(ini_hasSection(&ctx, nullptr));
    EXPECT_FALSE(ini_hasKey(&ctx, "section1", nullptr));
    
    char value[256];
    EXPECT_FALSE(ini_getValue(&ctx, "section1", "key1", nullptr, sizeof(value)));
    EXPECT_FALSE(ini_getValue(&ctx, "section1", "key1", value, 0));
}

TEST_F(IniParserTest, MalformedLines) {
    const char* content = 
        "[section1\n"  // Missing closing bracket
        "key1\n"       // Missing value
        "=value1\n"    // Missing key
        "key2:value2\n"// Colon separator
        "[section2]\n"
        "key3=value3\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    EXPECT_TRUE(ini_hasSection(&ctx, "section2"));
    EXPECT_FALSE(ini_hasSection(&ctx, "section1"));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "section2", "key3", value, sizeof(value)));
    EXPECT_STREQ(value, "value3");
}

TEST_F(IniParserTest, EmptyValues) {
    const char* content = 
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

TEST_F(IniParserTest, MultiSectionOperations) {
    const char* content = 
        "[sectionA]\n"
        "key1=value1\n"
        "[sectionB]\n"
        "key1=value2\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    char value[256];
    EXPECT_TRUE(ini_getValue(&ctx, "sectionA", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value1");
    
    EXPECT_TRUE(ini_getValue(&ctx, "sectionB", "key1", value, sizeof(value)));
    EXPECT_STREQ(value, "value2");
}

TEST_F(IniParserTest, KeyWithoutSection) {
    const char* content = 
        "key1=value1\n"
        "[section1]\n"
        "key2=value2\n";
    
    ASSERT_TRUE(LoadIniContent(content));
    
    EXPECT_FALSE(ini_hasKey(&ctx, "", "key1"));
    EXPECT_TRUE(ini_hasKey(&ctx, "section1", "key2"));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}