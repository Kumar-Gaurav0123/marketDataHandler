#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "protocol/fix/fix_parser.hpp"
#include <vector>
#include <tuple>

using Field = std::tuple<int, std::string>;

static std::vector<Field> collect(const char* msg) {
    std::vector<Field> fields;
    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    size_t len = 0;
    while (msg[len]) ++len;

    protocol::fix::FixParser::parse(data, len,
        [&](int tag, const uint8_t* value, size_t vlen) noexcept {
            fields.emplace_back(tag, std::string(reinterpret_cast<const char*>(value), vlen));
        });
    return fields;
}

TEST_CASE("FixParser - single field") {
    // "35=D\x01"
    auto fields = collect("35=D\x01");
    REQUIRE(fields.size() == 1);
    CHECK(std::get<0>(fields[0]) == 35);
    CHECK(std::get<1>(fields[0]) == "D");
}

TEST_CASE("FixParser - multiple fields") {
    // "8=FIX.4.2\x01 35=D\x01 49=SENDER\x01"
    auto fields = collect("8=FIX.4.2\x01" "35=D\x01" "49=SENDER\x01");
    REQUIRE(fields.size() == 3);
    CHECK(std::get<0>(fields[0]) == 8);
    CHECK(std::get<1>(fields[0]) == "FIX.4.2");
    CHECK(std::get<0>(fields[1]) == 35);
    CHECK(std::get<1>(fields[1]) == "D");
    CHECK(std::get<0>(fields[2]) == 49);
    CHECK(std::get<1>(fields[2]) == "SENDER");
}

TEST_CASE("FixParser - empty message produces no fields") {
    auto fields = collect("");
    CHECK(fields.empty());
}

TEST_CASE("FixParser - malformed (no equals sign) returns early") {
    // "35D\x01" — no '='
    auto fields = collect("35D\x01");
    CHECK(fields.empty());
}

TEST_CASE("FixParser - missing SOH stops parsing") {
    // "35=D" — no SOH at end
    auto fields = collect("35=D");
    CHECK(fields.empty());
}

TEST_CASE("FixParser - numeric value field") {
    auto fields = collect("34=42\x01");
    REQUIRE(fields.size() == 1);
    CHECK(std::get<0>(fields[0]) == 34);
    CHECK(std::get<1>(fields[0]) == "42");
}
