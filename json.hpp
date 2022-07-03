#pragma once
#include <memory>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

class Val;

class Obj
{
  friend class Root;

private:
  std::unique_ptr<std::unordered_map<std::string_view, Val>> fields;

public:
  Obj();
  Obj(Obj &&);
  Obj(const Obj &);
  auto empty() const -> bool;
  auto operator=(Obj &&) -> Obj &;
  auto operator=(const Obj &) -> Obj &;
  auto operator()(std::string_view) const -> const Val &;
  auto size() const -> std::size_t;
};

class Arr
{
  friend class Root;

private:
  std::unique_ptr<std::vector<Val>> vals;

public:
  Arr();
  Arr(const Arr &);
  Arr(Arr &&);
  auto operator=(const Arr &) -> Arr &;
  auto operator=(Arr &&) -> Arr &;
  auto empty() const -> bool;
  auto operator[](std::size_t) const -> const Val &;
  auto size() const -> std::size_t;
  auto begin() const -> const Val *;
  auto end() const -> const Val *;
};

class Num
{
  friend class Root;

private:
  std::string_view str;

public:
  auto asDouble() const -> double;
  auto asFloat() const -> float;
  auto asInt32() const -> int32_t;
  auto asInt64() const -> int64_t;
  auto asUInt32() const -> uint32_t;
  auto asUInt64() const -> uint64_t;
};

class Val
{
  friend class Root;

private:
  std::variant<std::string_view, Num, Obj, Arr, bool, std::nullptr_t> data;

public:
  template <typename T>
  Val(T v) : data(std::move(v))
  {
  }
  auto asArr() const -> const Arr &;
  auto asBool() const -> bool;
  auto asDouble() const -> double;
  auto asFloat() const -> float;
  auto asInt32() const -> int32_t;
  auto asInt64() const -> int64_t;
  auto asNum() const -> Num;
  auto asObj() const -> const Obj &;
  auto asStr() const -> std::string;
  auto asUInt32() const -> uint32_t;
  auto asUInt64() const -> uint64_t;
  auto begin() const -> const Val *;
  auto end() const -> const Val *;
  auto isArr() const -> bool;
  auto isBool() const -> bool;
  auto isNull() const -> bool;
  auto isNum() const -> bool;
  auto isObj() const -> bool;
  auto isStr() const -> bool;
  auto operator()(std::string_view) const -> const Val &;
  auto operator[](size_t) const -> const Val &;
  auto size() const -> std::size_t;
  operator bool() const;
  operator double() const;
  operator float() const;
  operator int32_t() const;
  operator int64_t() const;
  operator std::string() const;
  operator uint32_t() const;
  operator uint64_t() const;
};

class Root
{
private:
  std::string json;
  size_t pos = 0;
  Obj root;

  auto arr() -> Arr;
  auto bool_() -> bool;
  auto char_(char) -> void;
  auto eof() const -> bool;
  auto isArr() const -> bool;
  auto isBool() const -> bool;
  auto isChar(char) const -> bool;
  auto isNull() const -> bool;
  auto isNum() const -> bool;
  auto isObj() const -> bool;
  auto isStr() const -> bool;
  auto isVal() const -> bool;
  auto null() -> std::nullptr_t;
  auto num() -> Num;
  auto obj() -> Obj;
  auto str() -> std::string_view;
  auto val() -> Val;
  auto whitespace() -> void;

public:
  Root(std::istream &);
  Root(std::string);
  auto empty() const -> bool;
  auto operator()(std::string_view) const -> const Val &;
  auto size() const -> std::size_t;
};
