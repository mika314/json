#include <catch2/catch.hpp>
#include <json/json.hpp>
#include <sstream>

TEST_CASE("Empty objest", "[json]")
{
  using namespace json;
  auto st = std::istringstream{"{}"};
  const auto r = Root{st};
  REQUIRE(r.empty());
}

TEST_CASE("Num fields", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": 314,
  "n2": 3.14,
  "n3": 217e-2
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 3);
  REQUIRE(!r("n1").isNull());
  REQUIRE(!r("n1").isObj());
  REQUIRE(!r("n1").isArr());
  REQUIRE(!r("n1").isStr());
  REQUIRE(!r("n1").isBool());
  REQUIRE(r("n1").isNum());
  REQUIRE(r("n1").asNum().asInt32() == 314);
  REQUIRE(r("n2").isNum());
  REQUIRE(r("n2").asNum().asFloat() == Approx(3.14f));
  REQUIRE(r("n3").isNum());
  REQUIRE(r("n3").asNum().asFloat() == Approx(2.17f));

  auto fiels = r.getFields();
  REQUIRE(fiels.size() == 3);
  REQUIRE(std::find(std::begin(fiels), std::end(fiels), "n1") != std::end(fiels));
  REQUIRE(std::find(std::begin(fiels), std::end(fiels), "n2") != std::end(fiels));
  REQUIRE(std::find(std::begin(fiels), std::end(fiels), "n3") != std::end(fiels));
}

TEST_CASE("Str fields", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": "abcd",
  "n2": "\n",
  "n3": "ab\u0032"
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 3);
  REQUIRE(r("n1").isStr());
  REQUIRE(r("n1").asStr() == "abcd");
  REQUIRE(static_cast<std::string>(r("n1")) == "abcd");
  REQUIRE(r("n2").isStr());
  REQUIRE(r("n2").asStr() == "\n");
  REQUIRE(static_cast<std::string>(r("n2")) == "\n");
  REQUIRE(r("n3").isStr());
  REQUIRE(r("n3").asStr() == "ab2");
  REQUIRE(static_cast<std::string>(r("n3")) == "ab2");
}

TEST_CASE("Arrays", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": ["abcd", "efg"]
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 1);
  REQUIRE(r("n1").isArr());
  REQUIRE(r("n1").asArr().size() == 2);
  REQUIRE(r("n1").asArr()[0].asStr() == "abcd");
  REQUIRE(r("n1").asArr()[1].asStr() == "efg");
  REQUIRE(r("n1").size() == 2);
  REQUIRE(!r("n1")[0].isNum());
  REQUIRE(!r("n1")[0].isObj());
  REQUIRE(!r("n1")[0].isArr());
  REQUIRE(!r("n1")[0].isBool());
  REQUIRE(!r("n1")[0].isNull());
  REQUIRE(r("n1")[0].isStr());
  REQUIRE(static_cast<std::string>(r("n1")[0]) == "abcd");
  REQUIRE(r("n1")[1].asStr() == "efg");
}

TEST_CASE("Nested objects", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": {"abcd": "efg"}
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 1);
  REQUIRE(r("n1").isObj());
  REQUIRE(r("n1").size() == 1);
  REQUIRE(r("n1")("abcd").asStr() == "efg");
  REQUIRE(r("n1")("abcd").isStr());
  REQUIRE(static_cast<std::string>(r("n1")("abcd")) == "efg");
}

TEST_CASE("Range for-loop", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": [1, 1, 1, 1]
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 1);
  REQUIRE(r("n1").isArr());
  REQUIRE(r("n1").size() == 4);
  auto cnt = 0;
  for (const auto &e : r("n1"))
  {
    REQUIRE(e.asInt32() == 1);
    REQUIRE(e.asUInt32() == 1);
    REQUIRE(e.asInt64() == 1);
    REQUIRE(e.asUInt64() == 1);
    ++cnt;
  }
  REQUIRE(cnt == 4);
}

TEST_CASE("Parse from string", "[json]")
{
  using namespace json;
  const auto r = Root{R"({
  "n1": "abcd",
  "n2": "efg",
  "n3": "hij"
})"};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 3);
  REQUIRE(r("n1").isStr());
  REQUIRE(r("n1").asStr() == "abcd");
  REQUIRE(static_cast<std::string>(r("n1")) == "abcd");
  REQUIRE(r("n2").isStr());
  REQUIRE(r("n2").asStr() == "efg");
  REQUIRE(static_cast<std::string>(r("n2")) == "efg");
  REQUIRE(r("n3").isStr());
  REQUIRE(r("n3").asStr() == "hij");
  REQUIRE(static_cast<std::string>(r("n3")) == "hij");
}

TEST_CASE("Null values", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": null,
  "n2": null,
  "n3": null
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 3);
  REQUIRE(r("n1").isNull());
  REQUIRE(r("n2").isNull());
  REQUIRE(r("n3").isNull());
}

TEST_CASE("Boolean values", "[json]")
{
  using namespace json;
  auto st = std::istringstream{R"({
  "n1": true,
  "n2": false,
  "n3": true
})"};
  const auto r = Root{st};
  REQUIRE(!r.empty());
  REQUIRE(r.size() == 3);
  REQUIRE(r("n1").isBool());
  REQUIRE(r("n1").asBool());
  REQUIRE(r("n2").isBool());
  REQUIRE(!r("n2").asBool());
  REQUIRE(r("n3").isBool());
  REQUIRE(r("n3").asBool());
}
