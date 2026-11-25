#include <gtest/gtest.h>

#include "x/uuid.h"

static constexpr size_t uuid_with_hyphen_size = 36;
static constexpr size_t uuid_without_hyphen_size = 32;

TEST(UuidTest, Default)
{
    const std::string uuid = x::Uuid().Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, WithHyphen)
{
    const std::string uuid = x::Uuid{.with_hyphen = true}.Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, WithoutHyphen)
{
    const std::string uuid = x::Uuid{.with_hyphen = false}.Random();
    EXPECT_EQ(uuid.size(), uuid_without_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_without_hyphen_size);
}

TEST(UuidTest, LowerCase)
{
    const std::string uuid = x::Uuid{.lower_case = true}.Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, UpperCase)
{
    const std::string uuid = x::Uuid{.lower_case = false}.Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t upper_case_count = 0;
    for (const char c : uuid)
    {
        if (std::isupper(c) || std::isdigit(c) || c == '-') ++upper_case_count;
    }
    EXPECT_EQ(upper_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, WithHyphenLowerCase)
{
    const std::string uuid = x::Uuid{.lower_case = true, .with_hyphen = true}.Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, WithHyphenUpperCase)
{
    const std::string uuid = x::Uuid{.lower_case = false, .with_hyphen = true}.Random();
    EXPECT_EQ(uuid.size(), uuid_with_hyphen_size);
    size_t upper_case_count = 0;
    for (const char c : uuid)
    {
        if (std::isupper(c) || std::isdigit(c) || c == '-') ++upper_case_count;
    }
    EXPECT_EQ(upper_case_count, uuid_with_hyphen_size);
}

TEST(UuidTest, WithoutHyphenLowerCase)
{
    const std::string uuid = x::Uuid{.lower_case = true, .with_hyphen = false}.Random();
    EXPECT_EQ(uuid.size(), uuid_without_hyphen_size);
    size_t lower_case_count = 0;
    for (const char c : uuid)
    {
        if (std::islower(c) || std::isdigit(c) || c == '-') ++lower_case_count;
    }
    EXPECT_EQ(lower_case_count, uuid_without_hyphen_size);
}

TEST(UuidTest, WithoutHyphenUpperCase)
{
    const std::string uuid = x::Uuid{.lower_case = false, .with_hyphen = false}.Random();
    EXPECT_EQ(uuid.size(), uuid_without_hyphen_size);
    size_t upper_case_count = 0;
    for (const char c : uuid)
    {
        if (std::isupper(c) || std::isdigit(c) || c == '-') ++upper_case_count;
    }
    EXPECT_EQ(upper_case_count, uuid_without_hyphen_size);
}
