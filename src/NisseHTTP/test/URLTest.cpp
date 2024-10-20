#include "gtest/gtest.h"
#include "URL.h"

TEST(URLTest, URLDefaultConstruct)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url;
    EXPECT_EQ("", url.href());
    EXPECT_EQ("", url.protocol());
    EXPECT_EQ("", url.origin());
    EXPECT_EQ("", url.host());
    EXPECT_EQ("", url.hostname());
    EXPECT_EQ("", url.port());
    EXPECT_EQ("", url.pathname());
    EXPECT_EQ("", url.query());
    EXPECT_EQ("", url.hash());
}

TEST(URLTest, URLConstructParts)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http", "thorsanvil.dev:8080", "/Plop/path/twin.stuff?ace=4#1234");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, URLConstructDirect)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, URLConstructCopy)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     orig("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234");
    ThorsAnvil::Nisse::NisseHTTP::URL     url(orig);
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", orig.href());
    EXPECT_EQ("http", orig.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", orig.origin());
    EXPECT_EQ("thorsanvil.dev:8080", orig.host());
    EXPECT_EQ("thorsanvil.dev", orig.hostname());
    EXPECT_EQ(":8080", orig.port());
    EXPECT_EQ("/Plop/path/twin.stuff", orig.pathname());
    EXPECT_EQ("?ace=4", orig.query());
    EXPECT_EQ("#1234", orig.hash());
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, URLConstructMove)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     orig("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234");
    ThorsAnvil::Nisse::NisseHTTP::URL     url(std::move(orig));
    EXPECT_EQ("", orig.href());
    EXPECT_EQ("", orig.protocol());
    EXPECT_EQ("", orig.origin());
    EXPECT_EQ("", orig.host());
    EXPECT_EQ("", orig.hostname());
    EXPECT_EQ("", orig.port());
    EXPECT_EQ("", orig.pathname());
    EXPECT_EQ("", orig.query());
    EXPECT_EQ("", orig.hash());
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
}

TEST(URLTest, URLConstructCopyAssign)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     orig("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234");
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://Bad.com");

    url = orig;
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", orig.href());
    EXPECT_EQ("http", orig.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", orig.origin());
    EXPECT_EQ("thorsanvil.dev:8080", orig.host());
    EXPECT_EQ("thorsanvil.dev", orig.hostname());
    EXPECT_EQ(":8080", orig.port());
    EXPECT_EQ("/Plop/path/twin.stuff", orig.pathname());
    EXPECT_EQ("?ace=4", orig.query());
    EXPECT_EQ("#1234", orig.hash());
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, URLConstructMoveAssign)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     orig("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234");
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://Bad.com");

    url = std::move(orig);
    EXPECT_EQ("", orig.href());
    EXPECT_EQ("", orig.protocol());
    EXPECT_EQ("", orig.origin());
    EXPECT_EQ("", orig.host());
    EXPECT_EQ("", orig.hostname());
    EXPECT_EQ("", orig.port());
    EXPECT_EQ("", orig.pathname());
    EXPECT_EQ("", orig.query());
    EXPECT_EQ("", orig.hash());
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoPath)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080?ace=4&mace=12#1234");

    EXPECT_EQ("http://thorsanvil.dev:8080?ace=4&mace=12#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("", url.pathname());
    EXPECT_EQ("?ace=4&mace=12", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoPathBadQuery)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080&ace=4&mace=12#1234");

    EXPECT_EQ("http://thorsanvil.dev:8080&ace=4&mace=12#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("", url.pathname());
    EXPECT_EQ("&ace=4&mace=12", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoPathNoQuery)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080#1234");

    EXPECT_EQ("http://thorsanvil.dev:8080#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("", url.pathname());
    EXPECT_EQ("", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoPathNoQueryNoHash)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080");
    EXPECT_EQ("http://thorsanvil.dev:8080", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("", url.pathname());
    EXPECT_EQ("", url.query());
    EXPECT_EQ("", url.hash());
}

TEST(URLTest, BadQuery)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080/Plop/path/twin.stuff&ace=4&mace=12#1234");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff&ace=4&mace=12#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("&ace=4&mace=12", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoQuery)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080/Plop/path/twin.stuff#1234");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("", url.query());
    EXPECT_EQ("#1234", url.hash());
}

TEST(URLTest, NoQueryNoHash)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080/Plop/path/twin.stuff");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("", url.query());
    EXPECT_EQ("", url.hash());
}

TEST(URLTest, NoHash)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4");
    EXPECT_EQ("http://thorsanvil.dev:8080/Plop/path/twin.stuff?ace=4", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev:8080", url.origin());
    EXPECT_EQ("thorsanvil.dev:8080", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ(":8080", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("", url.hash());
}

TEST(URLTest, NoPort)
{
    ThorsAnvil::Nisse::NisseHTTP::URL     url("http://thorsanvil.dev/Plop/path/twin.stuff?ace=4#1234");
    EXPECT_EQ("http://thorsanvil.dev/Plop/path/twin.stuff?ace=4#1234", url.href());
    EXPECT_EQ("http", url.protocol());
    EXPECT_EQ("http://thorsanvil.dev", url.origin());
    EXPECT_EQ("thorsanvil.dev", url.host());
    EXPECT_EQ("thorsanvil.dev", url.hostname());
    EXPECT_EQ("", url.port());
    EXPECT_EQ("/Plop/path/twin.stuff", url.pathname());
    EXPECT_EQ("?ace=4", url.query());
    EXPECT_EQ("#1234", url.hash());
}






