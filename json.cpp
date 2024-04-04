#include "json.hpp"
#include <sstream>

namespace json
{
  Obj::Obj() : fields(std::make_unique<std::unordered_map<std::string_view, Val>>()) {}

  static auto readFromStream(std::istream &) -> std::string;

  Root::Root(std::istream &st)
    : json(readFromStream(st)), root_([this]() -> Val::Data {
        if (isStr())
          return str();
        if (isNum())
          return num();
        if (isObj())
          return obj();
        if (isArr())
          return arr();
        if (isBool())
          return bool_();
        if (isNull())
          return null();
        throw std::runtime_error("Json parse error");
      }())
  {
  }

  Root::Root(std::string aJson)
    : json(std::move(aJson)), root_([this]() -> Val::Data {
        if (isStr())
          return str();
        if (isNum())
          return num();
        if (isObj())
          return obj();
        if (isArr())
          return arr();
        if (isBool())
          return bool_();
        if (isNull())
          return null();
        throw std::runtime_error("Json parse error");
      }())
  {
  }

  auto readFromStream(std::istream &st) -> std::string
  {
    std::ostringstream str;
    str << st.rdbuf();
    return str.str();
  }

  auto Root::obj() -> Obj
  {
    Obj ret;
    whitespace();
    char_('{');
    whitespace();
    if (isStr())
      for (;;)
      {
        const auto name = str();
        whitespace();
        char_(':');
        const auto v = val();
        const auto tmp = ret.fields->try_emplace(name, v);
        if (!tmp.second)
          throw std::runtime_error("Duplicate field");
        whitespace();
        if (!isChar(','))
          break;
        char_(',');
        whitespace();
        if (!isStr())
          throw std::runtime_error("Expected field name");
      }
    char_('}');
    return ret;
  }

  Obj::Obj(const Obj &other) : Obj()
  {
    *fields = *other.fields;
  }

  Obj::Obj(Obj &&other) : Obj()
  {
    *fields = std::move(*other.fields);
  }

  auto Obj::operator=(const Obj &other) -> Obj &
  {
    *fields = *other.fields;
    return *this;
  }

  auto Obj::operator=(Obj &&other) -> Obj &
  {
    *fields = std::move(*other.fields);
    return *this;
  }

  Arr::Arr() : vals(std::make_unique<std::vector<Val>>()) {}

  Arr::Arr(const Arr &other) : Arr()
  {
    *vals = *other.vals;
  }

  Arr::Arr(Arr &&other) : Arr()
  {
    *vals = std::move(*other.vals);
  }

  auto Arr::operator=(const Arr &other) -> Arr &
  {
    *vals = *other.vals;
    return *this;
  }

  auto Arr::operator=(Arr &&other) -> Arr &
  {
    *vals = std::move(*other.vals);
    return *this;
  }

  auto Root::char_(char ch) -> void
  {
    if (eof())
      throw std::runtime_error("Unexpected end of file");
    auto tmpCh = json[pos];
    if (tmpCh != ch)
    {
      std::ostringstream ss;
      ss << "Unexpected character '" << tmpCh << "' != '" << ch << "'";
      throw std::runtime_error(ss.str());
    }
    ++pos;
  }

  auto Root::whitespace() -> void
  {
    while (isChar(' ') || isChar('\t') || isChar('\n') || isChar('\r'))
      ++pos;
  }

  auto Root::isStr() const -> bool
  {
    return isChar('"');
  }

  static auto isHex(char ch) -> bool
  {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
  }

  auto Root::str() -> std::string_view
  {
    char_('"');
    auto start = pos;
    while (!eof())
    {
      if (isChar('\\'))
      {
        ++pos;
        if (eof())
          throw std::runtime_error("Unexpected end of file");
        switch (json[pos])
        {
        case '"':
        case '\\':
        case '/':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't': ++pos; break;
        case 'u':
          for (auto i = 0; i < 4; ++i)
          {
            ++pos;
            if (eof())
              throw std::runtime_error("Unexpected end of file");
            if (!isHex(json[pos]))
              throw std::runtime_error("Expected hex character");
          }
          break;
        default: throw std::runtime_error("Unexpected escape sequence");
        }
      }
      else if (isChar('"'))
      {
        ++pos;
        return std::string_view(json.data() + start, pos - start - 1);
      }
      else
        ++pos;
    }
    throw std::runtime_error("Unexpected end of file");
  }

  auto Root::val() -> Val
  {
    whitespace();
    if (isStr())
      return str();
    if (isNum())
      return num();
    if (isObj())
      return obj();
    if (isArr())
      return arr();
    if (isBool())
      return bool_();
    if (isNull())
      return null();
    throw std::runtime_error("Unexpected character");
  }

  auto Root::eof() const -> bool
  {
    return pos >= json.size();
  }

  auto Root::isNum() const -> bool
  {
    return !eof() && (isChar('-') || (json[pos] >= '0' && json[pos] <= '9'));
  }

  auto Root::num() -> Num
  {
    auto ret = Num{};
    auto start = pos;
    if (isChar('-'))
      ++pos;
    while (!eof() && ((json[pos] >= '0' && json[pos] <= '9')))
      ++pos;
    // fraction?
    if (isChar('.'))
    {
      ++pos;
      while (!eof() && ((json[pos] >= '0' && json[pos] <= '9')))
        ++pos;
    }
    // exponent?
    if (isChar('e') || isChar('E'))
    {
      ++pos;
      if (isChar('-') || isChar('+'))
        ++pos;
      while (!eof() && ((json[pos] >= '0' && json[pos] <= '9')))
        ++pos;
    }
    ret.str = std::string_view(json.data() + start, pos - start);
    return ret;
  }

  auto Root::isArr() const -> bool
  {
    return isChar('[');
  }

  auto Root::isVal() const -> bool
  {
    return isStr() || isNum() || isObj() || isArr();
  }

  auto Root::arr() -> Arr
  {
    Arr ret;
    char_('[');
    whitespace();
    if (isVal())
    {
      ret.vals->push_back(val());
      whitespace();
    }
    while (isChar(','))
    {
      char_(',');
      whitespace();
      if (isVal())
      {
        ret.vals->push_back(val());
        whitespace();
      }
    }
    char_(']');
    return ret;
  }

  auto Root::isChar(char ch) const -> bool
  {
    return !eof() && json[pos] == ch;
  }

  auto Root::isBool() const -> bool
  {
    return isChar('t') || isChar('f');
  }

  auto Root::bool_() -> bool
  {
    if (isChar('t'))
    {
      char_('t');
      char_('r');
      char_('u');
      char_('e');
      return true;
    }
    else if (isChar('f'))
    {
      char_('f');
      char_('a');
      char_('l');
      char_('s');
      char_('e');
      return false;
    }
    else
      throw std::runtime_error("Expect bool value");
  }

  auto Root::isNull() const -> bool
  {
    return isChar('n');
  }
  auto Root::null() -> std::nullptr_t
  {
    char_('n');
    char_('u');
    char_('l');
    char_('l');
    return nullptr;
  }

  auto Root::isObj() const -> bool
  {
    return isChar('{');
  }

  auto Obj::operator()(std::string_view val) const -> const Val &
  {
    static const auto null = Val{nullptr};
    auto it = fields->find(val);
    if (it == fields->end())
      return null;
    return it->second;
  }

  auto Obj::empty() const -> bool
  {
    return fields->empty();
  }

  auto Obj::size() const -> std::size_t
  {
    return fields->size();
  }

  auto Root::empty() const -> bool
  {
    return root_.empty();
  }

  auto Root::operator()(std::string_view field) const -> const Val &
  {
    return root_(field);
  }

  auto Root::operator[](size_t idx) const -> const Val &
  {
    return root_[idx];
  }

  auto Root::size() const -> std::size_t
  {
    return root_.size();
  }

  auto Arr::empty() const -> bool
  {
    return vals->empty();
  }

  auto Arr::operator[](std::size_t idx) const -> const Val &
  {
    return vals->at(idx);
  }

  auto Arr::size() const -> std::size_t
  {
    return vals->size();
  }

  auto Num::asDouble() const -> double
  {
    return std::stod(std::string(str));
  }

  auto Num::asFloat() const -> float
  {
    return std::stof(std::string(str));
  }

  auto Num::asInt32() const -> int32_t
  {
    int32_t ret;
    std::stringstream ss;
    ss << str;
    ss >> ret;
    return ret;
  }

  auto Num::asInt64() const -> int64_t
  {
    int64_t ret;
    std::stringstream ss;
    ss << str;
    ss >> ret;
    return ret;
  }

  auto Num::asUInt32() const -> uint32_t
  {
    uint32_t ret;
    std::stringstream ss;
    ss << str;
    ss >> ret;
    return ret;
  }

  auto Num::asUInt64() const -> uint64_t
  {
    uint64_t ret;
    std::stringstream ss;
    ss << str;
    ss >> ret;
    return ret;
  }

  auto Val::asArr() const -> const Arr &
  {
    if (!std::holds_alternative<Arr>(data))
      throw std::runtime_error("Not an array");
    return std::get<Arr>(data);
  }

  auto Val::asBool() const -> bool
  {
    if (!std::holds_alternative<bool>(data))
      throw std::runtime_error("Not a boolean");
    return std::get<bool>(data);
  }

  auto Val::asNum() const -> Num
  {
    if (!std::holds_alternative<Num>(data))
      throw std::runtime_error("Not a number");
    return std::get<Num>(data);
  }

  auto Val::asObj() const -> const Obj &
  {
    if (!std::holds_alternative<Obj>(data))
      throw std::runtime_error("Not an object");
    return std::get<Obj>(data);
  }

  auto Val::asStr() const -> std::string
  {
    if (!std::holds_alternative<std::string_view>(data))
      throw std::runtime_error("Not a string");
    // unescape the string_view
    std::string ret;
    auto str = std::get<std::string_view>(data);
    for (auto i = 0U; i < str.size(); ++i)
    {
      if (str[i] == '\\')
      {
        ++i;
        if (i == str.size())
          throw std::runtime_error("Invalid escape sequence");
        switch (str[i])
        {
        case '"': ret.push_back('"'); break;
        case '\\': ret.push_back('\\'); break;
        case '/': ret.push_back('/'); break;
        case 'b': ret.push_back('\b'); break;
        case 'f': ret.push_back('\f'); break;
        case 'n': ret.push_back('\n'); break;
        case 'r': ret.push_back('\r'); break;
        case 't': ret.push_back('\t'); break;
        case 'u': {
          if (i + 4 >= str.size())
            throw std::runtime_error("Invalid escape sequence");
          auto hex = std::string{str.substr(i + 1, 4)};
          i += 4;
          auto code = std::stoul(hex, nullptr, 16);
          ret.push_back(static_cast<char>(code));
          break;
        }
        default: throw std::runtime_error("Invalid escape sequence");
        }
      }
      else
        ret.push_back(str[i]);
    }
    return ret;
  }

  auto Val::isArr() const -> bool
  {
    return std::holds_alternative<Arr>(data);
  }

  auto Val::isBool() const -> bool
  {
    return std::holds_alternative<bool>(data);
  }

  auto Val::isNull() const -> bool
  {
    return std::holds_alternative<std::nullptr_t>(data);
  }

  auto Val::isNum() const -> bool
  {
    return std::holds_alternative<Num>(data);
  }

  auto Val::isObj() const -> bool
  {
    return std::holds_alternative<Obj>(data);
  }

  auto Val::isStr() const -> bool
  {
    return std::holds_alternative<std::string_view>(data);
  }

  auto Arr::begin() const -> const Val *
  {
    return vals->data();
  }

  auto Arr::end() const -> const Val *
  {
    return vals->data() + vals->size();
  }

  auto Val::operator()(std::string_view field) const -> const Val &
  {
    return asObj()(field);
  }

  auto Val::operator[](size_t idx) const -> const Val &
  {
    return asArr()[idx];
  }

  auto Val::size() const -> std::size_t
  {
    if (isArr())
      return asArr().size();
    else if (isObj())
      return asObj().size();
    throw std::runtime_error("Not an array or object");
  }

  auto Val::empty() const -> bool
  {
    return size() == 0;
  }

  Val::operator std::string() const
  {
    return asStr();
  }

  Val::operator bool() const
  {
    return asBool();
  }

  Val::operator double() const
  {
    return asNum().asDouble();
  }

  Val::operator float() const
  {
    return asNum().asFloat();
  }

  Val::operator int32_t() const
  {
    return asNum().asInt32();
  }

  Val::operator int64_t() const
  {
    return asNum().asInt64();
  }

  Val::operator uint32_t() const
  {
    return asNum().asUInt32();
  }

  Val::operator uint64_t() const
  {
    return asNum().asUInt64();
  }

  auto Val::asInt32() const -> int32_t
  {
    return asNum().asInt32();
  }

  auto Val::asInt64() const -> int64_t
  {
    return asNum().asInt64();
  }

  auto Val::asUInt32() const -> uint32_t
  {
    return asNum().asUInt32();
  }

  auto Val::asUInt64() const -> uint64_t
  {
    return asNum().asUInt64();
  }

  auto Val::asDouble() const -> double
  {
    return asNum().asDouble();
  }

  auto Val::asFloat() const -> float
  {
    return asNum().asFloat();
  }

  auto Val::begin() const -> const Val *
  {
    return asArr().begin();
  }

  auto Val::end() const -> const Val *
  {
    return asArr().end();
  }

  auto Obj::getFields() const -> std::vector<std::string_view>
  {
    std::vector<std::string_view> ret;
    for (auto &[key, val] : *fields)
      ret.push_back(key);
    return ret;
  }

  auto Root::getFields() const -> std::vector<std::string_view>
  {
    return root_.getFields();
  }

  auto Root::root() const -> Val
  {
    return root_;
  }

  auto Val::getFields() const -> std::vector<std::string_view>
  {
    return asObj().getFields();
  }
} // namespace json
