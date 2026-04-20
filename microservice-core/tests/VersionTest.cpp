#include <gtest/gtest.h>
#include "version.hpp"
#include <string>

TEST(VersionTest, MacroValuesAreValid)
{
    EXPECT_GT(CPP_HTTP_SERVER_VERSION_MAJOR, -1);
    EXPECT_GT(CPP_HTTP_SERVER_VERSION_MINOR, -1);
    EXPECT_GE(CPP_HTTP_SERVER_VERSION_PATCH, 0);
}

TEST(VersionTest, VersionStringFormat)
{
    std::string version = CPP_HTTP_SERVER_VERSION;
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find('.'), std::string::npos);
}

TEST(VersionTest, VersionStringMatchesMacros)
{
    std::string version = CPP_HTTP_SERVER_VERSION;
    std::string expected = std::to_string(CPP_HTTP_SERVER_VERSION_MAJOR) + "." +
                           std::to_string(CPP_HTTP_SERVER_VERSION_MINOR) + "." +
                           std::to_string(CPP_HTTP_SERVER_VERSION_PATCH);
    EXPECT_EQ(version, expected);
}