// The MIT License (MIT)

// Copyright (c) 2014 Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This file was generated with a script.
// Generated 2015-03-10 03:04:22.494000 UTC
// This header was generated with jsonpp v0.9.0 (revision 368afad)
// https://github.com/Rapptz/jsonpp

#ifndef JSONPP_SINGLE_INCLUDE_HPP
#define JSONPP_SINGLE_INCLUDE_HPP

// beginning of D:\GitHub\jsonpp\jsonpp\parser.hpp

// beginning of D:/GitHub/jsonpp/jsonpp/error.hpp

#include <string>
#include <exception>

namespace json {
inline namespace v1 {
class parser_error : public std::exception {
private:
    std::string error;
public:
    parser_error(const std::string& str, unsigned line, unsigned column):
        error("stdin:" + std::to_string(line) + ':' + std::to_string(column) + ": error: " + str) {}

    const char* what() const noexcept override {
        return error.c_str();
    }
};
} // v1
} // json

// end of D:/GitHub/jsonpp/jsonpp/error.hpp

// beginning of D:/GitHub/jsonpp/jsonpp/value.hpp

// beginning of D:/GitHub/jsonpp/jsonpp/type_traits.hpp

#ifndef JSON_TYPE_TRAITS_HPP
#define JSON_TYPE_TRAITS_HPP

#include <type_traits>

namespace json {
inline namespace v1 {
template<typename T>
struct identity {
    using type = T;
};

template<typename T>
using Identity = typename identity<T>::type;

template<typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<bool B>
using Bool = std::integral_constant<bool, B>;

template<typename T>
using Not = Bool<!T::value>;

template<typename Condition, typename Then, typename Else>
using If = typename std::conditional<Condition::value, Then, Else>::type;

template<typename... Args>
struct And : Bool<true> {};

template<typename T, typename... Args>
struct And<T, Args...> : If<T, And<Args...>, Bool<false>> {};

template<typename... Args>
struct Or : Bool<false> {};

template<typename T, typename... Args>
struct Or<T, Args...> : If<T, Bool<true>, Or<Args...>> {};

template<typename... Args>
using EnableIf = typename std::enable_if<And<Args...>::value, int>::type;

template<typename... Args>
using DisableIf = typename std::enable_if<Not<And<Args...>>::value, int>::type;

template<typename T>
struct is_bool : std::is_same<T, bool> {};

template<typename T>
struct is_number : And<std::is_arithmetic<T>, Not<is_bool<T>>> {};

using null = decltype(nullptr);

template<typename T>
struct is_null : std::is_same<T, null> {};

class value;

template<typename T>
struct is_value : std::is_same<T, value> {};

template<typename T, typename U = typename std::decay<T>::type>
struct is_string : Or<std::is_same<U, std::string>, std::is_same<U, const char*>, std::is_same<U, char*>> {};

struct has_to_json_impl {
    template<typename T, typename U = decltype(to_json(std::declval<T>()))>
    static is_value<U> test(int);

    template<typename...>
    static std::false_type test(...);
};

namespace detail {
using std::end;
using std::begin;

struct has_iterators_impl {
    template<typename T, typename B = decltype(begin(std::declval<T>())),
                         typename E = decltype(end(std::declval<T>()))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

template<typename T>
struct is_possible_key_type : Or<is_bool<T>, is_number<T>, is_string<T>, is_value<T>> {};

struct is_array_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename V = typename U::value_type,
                         typename S = decltype(std::declval<U>().shrink_to_fit()),
                         typename R = decltype(std::declval<U>().reserve(0))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

struct is_object_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename K = typename U::key_type,
                         typename V = typename U::mapped_type,
                         typename C = typename U::key_compare>
    static is_possible_key_type<K> test(int);
    template<typename...>
    static std::false_type test(...);
};
} // detail

template<typename T>
struct has_iterators : decltype(detail::has_iterators_impl::test<T>(0)) {};

template<typename T>
struct is_array : And<Or<decltype(detail::is_array_impl::test<T>(0)), std::is_array<T>>, has_iterators<T>, Not<is_string<T>>> {};

template<typename T>
struct is_object : And<decltype(detail::is_object_impl::test<T>(0)), has_iterators<T>> {};

template<typename T>
struct has_to_json : decltype(has_to_json_impl::test<T>(0)) {};

enum class type {
    null, string, boolean, number, array, object
};
} // v1
} // json

#endif // JSON_TYPE_TRAITS_HPP
// end of D:/GitHub/jsonpp/jsonpp/type_traits.hpp

// beginning of D:/GitHub/jsonpp/jsonpp/dump.hpp

// beginning of D:/GitHub/jsonpp/jsonpp/detail/unicode.hpp

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdexcept>
#include <system_error>
#include <cerrno>

namespace json {
inline namespace v1 {
namespace detail {
#if defined(_WIN32)
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t must be 16 bits");
    std::wstring temp;
    auto required_size = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), nullptr, 0);
    temp.resize(required_size);
    if(::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), &temp[0], required_size) == 0) {
        throw std::runtime_error("unable to convert from UTF-8 to UTF-16");
    }
    return { temp.begin(), temp.end() };
}
#else
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    std::u16string result;
    using size_type = decltype(utf8.size());
    std::invalid_argument invalid_utf8("Invalid UTF-8 string given");
    size_type i = 0;
    while(i < utf8.size()) {
        char32_t codepoint;
        size_type iterations = 0;
        unsigned char byte = utf8[i++];
        if(byte <= 0x7F) {
            codepoint = byte;
        }
        else if(byte <= 0xBF) {
            throw invalid_utf8;
        }
        else if(byte <= 0xDF) {
            codepoint = byte & 0x1F;
            iterations = 1;
        }
        else if(byte <= 0xEF) {
            codepoint = byte & 0x0F;
            iterations = 2;
        }
        else if(byte <= 0xF7) {
            codepoint = byte & 0x07;
            iterations = 3;
        }
        else {
            throw invalid_utf8;
        }

        for(size_type j = 0; j < iterations; ++j) {
            if(i == utf8.size()) {
                throw invalid_utf8;
            }
            unsigned char next_byte = utf8[i++];
            if(next_byte < 0x80 || next_byte > 0xBF) {
                throw invalid_utf8;
            }

            codepoint = (codepoint << 6) + (next_byte & 0x3F);
        }
        if(codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            throw invalid_utf8;
        }

        if(codepoint <= 0xFFFF) {
            result.push_back(codepoint);
        }
        else {
            codepoint -= 0x10000;
            result.push_back((codepoint >> 10) + 0xD800);
            result.push_back((codepoint & 0x3FF) + 0xDC00);
        }
    }

    return result;
}
#endif
} // detail
} // v1
} // json

// end of D:/GitHub/jsonpp/jsonpp/detail/unicode.hpp

#include <sstream>
#include <iosfwd>
#include <cmath>

namespace json {
inline namespace v1 {
struct format_options {
    enum : int {
        none = 0,
        allow_nan_inf = 1 << 0,
        minify = 1 << 1,
        escape_multi_byte = 1 << 2
    };

    format_options() noexcept = default;
    format_options(int indent, int flags = none, int precision = 6) noexcept: flags(flags), indent(indent), precision(precision) {}

    int flags = none;
    int indent = 4;
    int precision = 6;
    int depth = 0;
};

namespace detail {
template<typename OStream>
inline void indent(OStream& out, const format_options& opt) {
    out << '\n';
    for(int i = 0; i < (opt.indent * opt.depth); ++i) {
        out << ' ';
    }
}
} // detail

template<typename OStream, typename T, EnableIf<is_null<T>> = 0>
inline OStream& dump(OStream& out, const T&, format_options = {}) {
    out << "null";
    return out;
}

template<typename OStream, typename T, EnableIf<is_bool<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options = {}) {
    out << (t ? "true" : "false");
    return out;
}

template<typename OStream, typename T, EnableIf<is_number<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    if((opt.flags & opt.allow_nan_inf) != opt.allow_nan_inf && (std::isnan(t) || std::isinf(t))) {
        // stream null instead if nan is found
        out << "null";
        return out;
    }
    auto precision = out.precision();
    out.precision(opt.precision);
    out << t;
    out.precision(precision);
    return out;
}

namespace detail {
template<typename OStream>
inline bool escape_control(OStream& out, char control) {
    switch(control) {
    case '"':
        out << "\\\"";
        return true;
    case '\\':
        out << "\\\\";
        return true;
    case '/':
        out << "\\/";
        return true;
    case '\b':
        out << "\\b";
        return true;
    case '\f':
        out << "\\f";
        return true;
    case '\n':
        out << "\\n";
        return true;
    case '\r':
        out << "\\r";
        return true;
    case '\t':
        out << "\\t";
        return true;
    default:
        return false;
    }
}

template<typename OStream>
inline OStream& escape_str(OStream& out, const std::u16string& utf16) {
    auto fill = out.fill();
    auto width = out.width();
    auto flags = out.flags();
    out << '"';
    for(auto&& c : utf16) {
        if(c <= 0x7F) {
            if(escape_control(out, static_cast<char>(c))) {
                continue;
            }
            out << static_cast<char>(c);
        }
        else {
            // prepare stream
            out << "\\u";
            out.width(4);
            out.fill('0');
            out.setf(out.hex, out.basefield);
            out << c;
            out.width(width);
            out.fill(fill);
            out.flags(flags);
        }
    }
    out << '"';
    return out;
}
} // detail

template<typename OStream, typename T, EnableIf<is_string<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool escape = (opt.flags & opt.escape_multi_byte) == opt.escape_multi_byte;
    if(escape) {
        return detail::escape_str(out, detail::utf8_to_utf16(t));
    }
    out << '"';
    for(auto&& c : t) {
        if(detail::escape_control(out, c)) {
            continue;
        }
        else {
            out << c;
        }
    }
    out << '"';
    return out;
}

template<typename OStream, typename T, EnableIf<is_array<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool prettify = (opt.flags & opt.minify) != opt.minify;
    opt.depth += prettify;
    out << '[';

    using std::begin;
    using std::end;
    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;
    for(; first != last; ++first) {
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        dump(out, *first, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;
        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << ']';
    return out;
}

template<typename OStream, typename T, EnableIf<std::is_arithmetic<T>> = 0>
inline void key(OStream& out, const T& t, const format_options&) {
    out << '"' << std::to_string(t) << '"';
}

template<typename OStream, typename T, DisableIf<std::is_arithmetic<T>> = 0>
inline void key(OStream& out, const T& t, const format_options& opt) {
    dump(out, t, opt);
}

template<typename OStream, typename T, EnableIf<is_object<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool prettify = (opt.flags & format_options::minify) != format_options::minify;
    opt.depth += prettify;
    out << '{';

    using std::begin;
    using std::endl;

    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;

    for(; first != last; ++first) {

        auto&& elem = *first;
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        key(out, elem.first, opt);
        out << ':';

        if(prettify) {
            out << ' ';
        }

        dump(out, elem.second, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;

        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << '}';
    return out;
}

template<typename OStream, typename T, EnableIf<has_to_json<T>, Not<Or<is_object<T>, is_string<T>, is_array<T>>>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    return dump(out, to_json(t), opt);
}

template<typename T>
inline std::string dump_string(const T& value, format_options opt = {}) {
    std::ostringstream ss;
    dump(ss, value, opt);
    return ss.str();
}
} // v1
} // json

// end of D:/GitHub/jsonpp/jsonpp/dump.hpp

#include <map>
#include <vector>
#include <cassert>
#include <cstdint>
#include <memory>

namespace json {
inline namespace v1 {
class value {
public:
    using object = std::map<std::string, value>;
    using array  = std::vector<value>;
private:
    union storage_t {
        double number;
        bool boolean;
        std::string* str;
        array* arr;
        object* obj;
    } storage;
    type storage_type;

    void copy(const value& other) {
        switch(other.storage_type) {
        case type::array:
            storage.arr = new array(*(other.storage.arr));
            break;
        case type::string:
            storage.str = new std::string(*(other.storage.str));
            break;
        case type::object:
            storage.obj = new object(*(other.storage.obj));
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        default:
            break;
        }
        storage_type = other.storage_type;
    }
public:
    value() noexcept: storage_type(type::null) {}
    value(null) noexcept: storage_type(type::null) {}

    ~value() {
        clear();
    }

    value(double v) noexcept: storage_type(type::number) {
        storage.number = v;
    }

    template<typename T, EnableIf<is_bool<T>, Not<is_string<T>>> = 0>
    value(const T& b) noexcept: storage_type(type::boolean) {
        storage.boolean = b;
    }

    template<typename T, EnableIf<is_string<T>, Not<is_bool<T>>> = 0>
    value(const T& str): storage_type(type::string) {
        storage.str = new std::string(str);
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value(const T& t): value(to_json(t)) {}

    value(const array& arr): storage_type(type::array) {
        storage.arr = new array(arr);
    }

    value(const object& obj): storage_type(type::object) {
        storage.obj = new object(obj);
    }

    value(std::initializer_list<array::value_type> l): storage_type(type::array) {
        storage.arr = new array(l.begin(), l.end());
    }

    value(const value& other) {
        copy(other);
    }

    value(value&& other) noexcept {
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value& operator=(const T& t) {
        *this = to_json(t);
        return *this;
    }

    value& operator=(const value& other) noexcept {
        clear();
        copy(other);
        return *this;
    }

    value& operator=(value&& other) {
        clear();
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
        return *this;
    }

    std::string type_name() const {
        switch(storage_type) {
        case type::array:
            return "array";
        case type::string:
            return "string";
        case type::object:
            return "object";
        case type::number:
            return "number";
        case type::boolean:
            return "boolean";
        case type::null:
            return "null";
        default:
            return "unknown";
        }
    }

    void clear() noexcept {
        switch(storage_type) {
        case type::array:
            delete storage.arr;
            break;
        case type::string:
            delete storage.str;
            break;
        case type::object:
            delete storage.obj;
            break;
        default:
            break;
        }
        storage_type = type::null;
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    bool is() const noexcept {
        return storage_type == type::string;
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    bool is() const noexcept {
        return storage_type == type::null;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    bool is() const noexcept {
        return storage_type == type::number;
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    bool is() const noexcept {
        return storage_type == type::boolean;
    }

    template<typename T, EnableIf<std::is_same<T, object>> = 0>
    bool is() const noexcept {
        return storage_type == type::object;
    }

    template<typename T, EnableIf<std::is_same<T, array>> = 0>
    bool is() const noexcept {
        return storage_type == type::array;
    }

    template<typename T, EnableIf<std::is_same<T, const char*>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return storage.str->c_str();
    }

    template<typename T, EnableIf<std::is_same<T, std::string>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return *(storage.str);
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return {};
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return storage.boolean;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return storage.number;
    }

    template<typename T, EnableIf<std::is_same<T, object>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return *(storage.obj);
    }

    template<typename T, EnableIf<std::is_same<T, array>> = 0>
    T as() const noexcept {
        assert(is<T>());
        return *(storage.arr);
    }

    template<typename T>
    T as(Identity<T>&& def) const noexcept {
        return is<T>() ? as<T>() : std::forward<T>(def);
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    value operator[](const T& str) const noexcept {
        if(!is<object>()) {
            return {};
        }

        auto it = storage.obj->find(str);
        if(it != storage.obj->end()) {
            return it->second;
        }
        return {};
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    value operator[](const T& index) const noexcept {
        if(!is<array>()) {
            return {};
        }

        auto&& arr = *storage.arr;

        if(static_cast<size_t>(index) < arr.size()) {
            return arr[index];
        }
        return {};
    }

    template<typename OStream>
    friend OStream& dump(OStream& out, const value& val, format_options opt = {}) {
        switch(val.storage_type) {
        case type::array:
            return dump(out, *val.storage.arr, opt);
        case type::string:
            return dump(out, *val.storage.str, opt);
        case type::object:
            return dump(out, *val.storage.obj, opt);
        case type::boolean:
            return dump(out, val.storage.boolean, opt);
        case type::number:
            return dump(out, val.storage.number, opt);
        case type::null:
            return dump(out, nullptr, opt);
        default:
            return out;
        }
    }
};

using array  = value::array;
using object = value::object;
} // v1
} // json

// end of D:/GitHub/jsonpp/jsonpp/value.hpp

#include <cstring>

namespace json {
inline namespace v1 {
inline bool is_space(char ch) {
    switch(ch) {
    case 0x0D: // carriage return
    case 0x09: // tab
    case 0x0A: // line feed
    case 0x20: // space
        return true;
    default:
        return false;
    }
}

struct parser {
private:
    unsigned line = 1;
    unsigned column = 1;
    const char* str;

    void skip_white_space() {
        while(*str != '\0' && is_space(*str)) {
            if(*str == 0x0A) {
                ++line;
                column = 0;
            }
            ++str;
            ++column;
        }
    }

    void parse_null(value& v) {
        static const char null_str[] = "null";
        if(*str == '\0') {
            throw parser_error("expected null, received EOF instead", line, column);
        }

        if(std::strncmp(str, null_str, sizeof(null_str) - 1) != 0) {
            throw parser_error("expected null not found", line, column);
        }

        v = nullptr;
        str = str + sizeof(null_str) - 1;
        column += sizeof(null_str);
    }

    void parse_number(value& v) {
        static const std::string lookup = "0123456789eE+-.";
        const char* begin = str;
        if(*begin == '\0') {
            throw parser_error("expected number, received EOF instead", line, column);
        }

        while(lookup.find(*str) != std::string::npos) {
            ++str;
        }

        double val = 0.0;
        try {
            std::string temp(begin, str);
            val = std::stod(temp);
            column += temp.size() + 1;
        }
        catch(const std::exception& e) {
            throw parser_error("number could not be parsed properly", line, column);
        }

        v = val;
    }

    int get_codepoint(const char*& copy) {
        int codepoint = 0;
        for(unsigned i = 0; i < 4; ++i) {
            char hex = *copy;
            if(hex <= 0x1F) {
                throw parser_error("incomplete codepoint provided", line, column);
            }

            // convert the hex character to its integral representation
            // e.g., 'F' -> 15
            if(hex >= '0' && hex <= '9') {
                hex -= '0';
            }
            else if(hex >= 'A' && hex <= 'F') {
                hex -= 'A' - 0xA;
            }
            else if(hex >= 'a' && hex <= 'f') {
                hex -= 'a' - 0xA;
            }
            else {
                throw parser_error("invalid codepoint provided", line, column);
            }

            codepoint = codepoint * 16 + hex;
            ++column;
            ++copy;
        }
        return codepoint;
    }

    void parse_codepoint(const char*& copy, std::string& result) {
        // parse the hex characters
        // in order to do so, we have to increment by one to get these digits.
        // ergo, *copy == 'u', ++copy = codepoint
        ++copy;
        ++column;
        int codepoint = get_codepoint(copy);

        // a regular ASCII code point
        if(codepoint < 0x80) {
            result.push_back(codepoint);
            return;
        }

        // handle surrogate pairs
        if(codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            if(codepoint >= 0xDC00) {
                throw parser_error("low surrogate pair found but high surrogate pair expected", line, column);
            }

            // get the  low surrogate pair
            if(*(copy + 1) != '\\' && *(copy + 2) != 'u') {
                throw parser_error("low surrogate pair expected but not found", line, column);
            }

            copy += 2;
            int low_surrogate = get_codepoint(copy);
            if(low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                throw parser_error("low surrogate out of range [\\uDC000, \\uDFFF]", line, column);
            }

            codepoint = 0x10000 + (((codepoint - 0xD800) << 10) | ((low_surrogate - 0xDC00) & 0x3FF));
        }

        if(codepoint < 0x800) {
            result.push_back(0xC0 | (codepoint >> 6));
        }
        else if(codepoint < 0x10000) {
            result.push_back(0xE0 | (codepoint >> 12));
        }
        else {
            result.push_back(0xF0 | (codepoint >> 18));
            result.push_back(0x80 | ((codepoint >> 12) & 0x3F));
        }
        result.push_back(0x80 | ((codepoint >> 6) & 0x3F));
        result.push_back(0x80 | (codepoint & 0x3F));
    }

    template<typename Value>
    void parse_string(Value& v) {
        const char* copy = str + 1;
        if(*copy == '\0') {
            throw parser_error("expected string, received EOF instead", line, column);
        }

        std::string result;

        // begin parsing
        while(true) {
            ++column;
            bool increment_string = true;
            auto byte = static_cast<unsigned char>(*copy);

            if(byte <= 0x1F) {
                throw parser_error("invalid characters found in string or string is incomplete", line, column);
            }

            // end of string found
            if(*copy == '"') {
                break;
            }

            // non-escape character is good to go
            if(*copy != '\\') {
                result.push_back(*copy++);
                continue;
            }

            // at this point *copy == '\\'
            // so increment it to check the next character
            ++copy;
            switch(*copy) {
            case '/':
                result.push_back('/');
                break;
            case '\\':
                result.push_back('\\');
                break;
            case '"':
                result.push_back('\"');
                break;
            case 'f':
                result.push_back('\f');
                break;
            case 'n':
                result.push_back('\n');
                break;
            case 'r':
                result.push_back('\r');
                break;
            case 't':
                result.push_back('\t');
                break;
            case 'b':
                result.push_back('\b');
                break;
            case 'u':
                parse_codepoint(copy, result);
                increment_string = false;
                break;
            default:
                throw parser_error("improper or incomplete escape character found", line, column);
            }

            if(increment_string) {
                ++copy;
            }
        }

        v = result;
        ++copy;
        str = copy;
    }

    void parse_bool(value& v) {
        if(*str == '\0') {
            throw parser_error("expected boolean, received EOF instead", line, column);
        }

        bool expected_true = *str == 't';
        const char* boolean = expected_true ? "true" : "false";
        const size_t len = expected_true ? 4 : 5;

        if(std::strncmp(str, boolean, len) != 0) {
            throw parser_error("expected boolean not found", line, column);
        }

        v = expected_true;
        str = str + len;
        column += len;
    }

    void parse_array(value& v) {
        ++str;
        array arr;
        value elem;
        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected value, received EOF instead", line, column);
        }

        while (*str && *str != ']') {
            parse_value(elem);
            if(*str != ',') {
                if(*str != ']') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                ++str;
                // skip whitespace
                skip_white_space();
                // handle missing input
                if(*str && *str == ']') {
                    throw parser_error("extraneous comma spotted", line, column);
                }
            }

            arr.push_back(elem);
        }

        v = arr;
        if(*str == ']') {
            ++str;
        }
    }

    void parse_object(value& v) {
        ++str;
        object obj;
        std::string key;
        value elem;

        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected string key, received EOF instead", line, column);
        }

        while(*str) {
            skip_white_space();

            // empty object
            if(*str == '}') {
                break;
            }

            if(*str != '"') {
                throw parser_error("expected string as key not found", line, column);
            }
            parse_string(key);
            skip_white_space();

            if(*str != ':') {
                throw parser_error("missing semicolon", line, column);
            }
            ++str;
            parse_value(elem);

            if(*str != ',') {
                if(*str != '}') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                ++str;
            }
            obj.emplace(key, elem);
        }

        v = obj;
        if(*str == '}') {
            ++str;
        }
    }

    void parse_value(value& v) {
        skip_white_space();
        if(*str == '\0') {
            throw parser_error("unexpected EOF found", line, column);
        }

        if(isdigit(*str) || *str == '+' || *str == '-') {
            parse_number(v);
        }
        else {
            switch(*str) {
            case 'n':
                parse_null(v);
                break;
            case '"':
                parse_string(v);
                break;
            case 't':
            case 'f':
                parse_bool(v);
                break;
            case '[':
                parse_array(v);
                break;
            case '{':
                parse_object(v);
                break;
            default:
                throw parser_error("unexpected token found", line, column);
                break;
            }
        }

        skip_white_space();
    }
public:
    parser(const char* str) noexcept: str(str) {}

    void parse(value& v) {
        parse_value(v);
        if(*str != '\0') {
            throw parser_error("unexpected token found", line, column);
        }
    }
};

inline void parse(const std::string& str, value& v) {
    parser js(str.c_str());
    js.parse(v);
}

template<typename IStream, DisableIf<is_string<IStream>> = 0>
inline void parse(IStream& in, value& v) {
    static_assert(std::is_base_of<std::istream, IStream>::value, "Input stream passed must inherit from std::istream");
    if(in) {
        std::ostringstream ss;
        ss << in.rdbuf();
        parse(ss.str(), v);
    }
}
} // v1
} // json

// end of D:\GitHub\jsonpp\jsonpp\parser.hpp

#endif // JSONPP_SINGLE_INCLUDE_HPP
