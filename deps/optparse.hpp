// The MIT License (MIT)

// Copyright (c) 2012-2014 Danny Y., Rapptz

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
// Generated 2015-03-09 23:29:00.617000 UTC
// This header is part of gears v0.9.3 (revision 8dc0283)
// https://github.com/Rapptz/Gears

#ifndef GEARS_SINGLE_INCLUDE_OPTPARSE_HPP
#define GEARS_SINGLE_INCLUDE_OPTPARSE_HPP

// beginning of gears\optparse.hpp

// beginning of gears\optparse/option_parser.hpp

// beginning of gears\optparse/error.hpp

#include <string>
#include <stdexcept>

namespace gears {
namespace optparse {



class error : public std::runtime_error {
public:
    std::string program_name;
    std::string option_name;
    std::string error_string;

    error(const std::string& name, const std::string& str, const std::string& op):
        std::runtime_error(name + ": error: " + str), program_name(name), option_name(op), error_string(str) {}
};


class unrecognised_option : public error {
public:

    unrecognised_option(const std::string& name, const std::string& op):
    error(name, "unrecognised option '" + op + '\'', op) {}
};


class missing_required_option : public error {
public:

    missing_required_option(const std::string& name, const std::string& op):
    error(name, "missing required option '" + op + '\'', op) {}
};


class missing_required_value : public error {
public:

    missing_required_value(const std::string& name, const std::string& op, size_t nargs):
    error(name, nargs == 1 ?
                "option '" + op + "' requires an argument" :
                "option '" + op + "' requires " + std::to_string(nargs) + " arguments", op) {}
};


class option_takes_no_value : public error {
public:

    option_takes_no_value(const std::string& name, const std::string& op):
    error(name, "option '" + op + "' does not take a value", op) {}
};
} // optparse
} // gears

// end of gears\optparse/error.hpp

// beginning of gears\optparse/formatter.hpp

// beginning of gears\optparse/subcommand.hpp

// beginning of gears\optparse/option_set.hpp

// beginning of gears\optparse/option.hpp

// beginning of gears\optparse/value.hpp

// beginning of gears\optparse/actions.hpp

// beginning of gears\string/lexical_cast.hpp

#include <string>
#include <sstream>
#include <stdexcept>

namespace gears {
namespace string {
namespace detail {
template<typename Target>
struct lexical_caster {
    template<typename CharT>
    static inline Target cast(const std::basic_string<CharT>& str) {
        std::basic_istringstream<CharT> ss(str);
        Target result;
        if((ss >> result).fail() || !(ss >> std::ws).eof()) {
            throw std::invalid_argument("lexical_cast failed");
        }
        return result;
    }
};

template<typename CharT>
struct lexical_caster<std::basic_string<CharT>> {
    static inline std::basic_string<CharT> cast(const std::basic_string<CharT>& str) {
        return str;
    }
};

template<>
struct lexical_caster<int> {
    template<typename CharT>
    static inline int cast(const std::basic_string<CharT>& str) {
        return std::stoi(str);
    }
};

template<>
struct lexical_caster<long> {
    template<typename CharT>
    static inline long cast(const std::basic_string<CharT>& str) {
        return std::stol(str);
    }
};

template<>
struct lexical_caster<long long> {
    template<typename CharT>
    static inline long long cast(const std::basic_string<CharT>& str) {
        return std::stoll(str);
    }
};

template<>
struct lexical_caster<float> {
    template<typename CharT>
    static inline float cast(const std::basic_string<CharT>& str) {
        return std::stof(str);
    }
};

template<>
struct lexical_caster<double> {
    template<typename CharT>
    static inline double cast(const std::basic_string<CharT>& str) {
        return std::stod(str);
    }
};

template<>
struct lexical_caster<long double> {
    template<typename CharT>
    static inline long double cast(const std::basic_string<CharT>& str) {
        return std::stold(str);
    }
};

template<>
struct lexical_caster<unsigned long> {
    template<typename CharT>
    static inline unsigned long cast(const std::basic_string<CharT>& str) {
        return std::stoul(str);
    }
};

template<>
struct lexical_caster<unsigned long long> {
    template<typename CharT>
    static inline unsigned long long cast(const std::basic_string<CharT>& str) {
        return std::stoull(str);
    }
};

template<typename T>
struct remove_const {
    using type = T;
};

template<typename T>
struct remove_const<const T> {
    using type = T;
};

template<typename T>
struct remove_volatile {
    using type = T;
};

template<typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

template<typename T>
struct remove_cv {
    using type = typename std::remove_volatile<typename std::remove_const<T>::type>::type;
};
} // detail


template<typename Target, typename CharT>
inline Target lexical_cast(const std::basic_string<CharT>& str) {
    return detail::lexical_caster<typename detail::remove_cv<Target>::type>::cast(str);
}
} // string
} // gears

// end of gears\string/lexical_cast.hpp

#include <functional>
#include <iterator>

namespace gears {
namespace optparse {



template<typename T>
struct store {

    T operator()(const std::string&, const std::string& value) const {
        return string::lexical_cast<T>(value);
    }
};

template<>
struct store<bool> {
    bool operator()(const std::string& key, const std::string& value) const {
        if(value == "1" || value == "true") {
            return true;
        }
        else if(value == "0" || value == "false") {
            return false;
        }

        throw std::runtime_error("invalid boolean argument passed to '" + key + '\'');
    }
};


template<typename T>
struct store_const {
private:
    T value;
public:

    store_const(T val): value(std::move(val)) {}


    T operator()(const std::string&, const std::string&) const {
        return value;
    }
};


template<typename Container, typename Action = store<typename Container::value_type>>
struct store_list {
    static_assert(std::is_convertible<decltype(std::declval<Action>()("","")), typename Container::value_type>::value,
                  "The action must return a type convertible to the container's value type");
private:
    Action action;
public:

    store_list(Action action): action(std::move(action)) {}

    Container operator()(const std::string& key, const std::string& value) const {
        Container result;
        std::istringstream ss(value);
        std::insert_iterator<Container> it(result, result.end());
        for(std::string str; std::getline(ss, str); ) {
            *it = action(key, str);
        }
        return result;
    }
};


template<typename Container>
struct append {
private:
    using value_type = typename Container::value_type;
    std::function<value_type(const std::string&, const std::string&)> action;
    Container cont;
public:
    template<typename Action>
    append(Action action): action(std::move(action)) {}


    Container operator()(const std::string& key, const std::string& value) {
        cont.insert(cont.end(), action(key, value));
        return cont;
    }
};
} // optparse
} // gears

// end of gears\optparse/actions.hpp

// beginning of gears\utility/helpers.hpp

// beginning of gears\meta/indices.hpp

#include <cstddef>

namespace gears {
namespace meta {

template<typename T, T... Integers>
struct integer_sequence {
    using value_type = T;


    static constexpr size_t size() noexcept {
        return sizeof...(Integers);
    }
};

namespace detail {
template<size_t... I>
struct indices {
    using type = indices;
};

template<typename Left, typename Right>
struct concat;

template<size_t... I, size_t... J>
struct concat<indices<I...>, indices<J...>> : indices<I..., (J + sizeof...(I))...> {};

template<size_t N>
struct build_indices : concat<typename build_indices<N / 2>::type, typename build_indices<N - N / 2>::type> {};

template<>
struct build_indices<0> : indices<> {};

template<>
struct build_indices<1> : indices<0> {};

template<typename T, T N, typename X = typename build_indices<N>::type>
struct generator;

template<typename T, T N, size_t... Indices>
struct generator<T, N, indices<Indices...>> {
    static_assert(N >= 0, "integer sequence must be positive");
    using type = integer_sequence<T, static_cast<T>(Indices)...>;
};
} // detail


template<typename T, T Max>
using make_integer_sequence = typename detail::generator<T, Max>::type;

template<size_t Max>
using make_index_sequence = typename detail::generator<size_t, Max>::type;

template<typename... Args>
using index_sequence_for = make_index_sequence<sizeof...(Args)>;

template<size_t... Indices>
using index_sequence = integer_sequence<size_t, Indices...>;
} // meta
} // gears

// end of gears\meta/indices.hpp

#include <array>
#include <utility>
#include <memory>
#include <type_traits>

namespace gears {
namespace detail {
template<typename T>
struct unique_type {
    using single = std::unique_ptr<T>;
};

template<typename T>
struct unique_type<T[]> {
    using array = std::unique_ptr<T[]>;
};

template<typename T, size_t N>
struct unique_type<T[N]> {
    using unknown = void;
};

template<typename CharT, size_t N, size_t... Indices>
constexpr std::array<CharT, N> literal_to_array(const CharT (&arr)[N], meta::index_sequence<Indices...>) {
    return std::array<CharT, N>{{ arr[Indices]... }};
}
} // detail


template<typename T, typename... Args>
constexpr std::array<T, sizeof...(Args)> make_array(Args&&... args) {
    return std::array<T, sizeof...(Args)>{{ std::forward<Args>(args)... }};
}


template<typename CharT, size_t N>
constexpr std::array<CharT, N> make_array(const CharT (&str)[N]) {
    return detail::literal_to_array(str, meta::make_index_sequence<N>{});
}


template<typename T, typename... Args>
inline typename detail::unique_type<T>::single make_unique(Args&&... args) {
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}


template<typename T>
inline typename detail::unique_type<T>::array make_unique(size_t size) {
    using U = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>{new U[size]()};
}

template<typename T, typename... Args>
inline typename detail::unique_type<T>::unknown make_unique(Args&&...) = delete;
} // gears

// end of gears\utility/helpers.hpp

namespace gears {
namespace optparse {
struct value_base {
    std::string metavar;     ///< The value name to use in help messages
    size_t nargs = 1;        ///< Number of arguments to expect.

    virtual ~value_base() = default;
    virtual void parse(const std::string&, const std::string&) = 0;
    virtual bool is_active() const noexcept = 0;
    virtual std::unique_ptr<value_base> clone() const = 0;
};


template<typename T>
struct typed_value : public value_base {
private:
    T* reference = nullptr;
    std::unique_ptr<T> value;
    std::function<T(const std::string&, const std::string&)> action_;
    bool active = false;

    void parse(const std::string& key, const std::string& val) override {
        auto&& result = action_(key, val);
        active = true;
        if(reference != nullptr) {
            *reference = std::move(result);
        }
        else {
            value.reset(new T(std::move(result)));
        }
    }

    std::unique_ptr<value_base> clone() const override {
        auto&& ptr = ::gears::make_unique<typed_value<T>>(action_);
        ptr->reference = reference;
        ptr->active = active;
        ptr->nargs = nargs;
        ptr->metavar = metavar;
        return std::unique_ptr<value_base>{std::move(ptr)};
    }
public:

    template<typename Action>
    typed_value(Action action): action_(std::move(action)) {}


    typed_value(T& object): reference(std::addressof(object)) {}


    bool is_active() const noexcept override {
        return active;
    }


    template<typename Action>
    typed_value& action(Action&& action) {
        action_ = std::forward<Action>(action);
        return *this;
    }


    const T& get() const {
        if(!active) {
            throw std::runtime_error("value has not been parsed");
        }

        if(reference != nullptr) {
            return *reference;
        }
        return *value;
    }


    const T& get_or(const T& def) const noexcept {
        if(is_active()) {
            if(reference != nullptr) {
                return *reference;
            }
            return *value;
        }
        return def;
    }
};


template<typename Container, typename Action = store<typename Container::value_type>>
inline std::unique_ptr<value_base> compose(std::string metavar = "", Action action = Action{}) {
    auto&& ptr = ::gears::make_unique<typed_value<Container>>(append<Container>{action});
    ptr->metavar = std::move(metavar);
    return std::unique_ptr<value_base>{std::move(ptr)};
}


template<typename T, typename Action = store<T>>
inline std::unique_ptr<value_base> bind_to(T& t, std::string metavar = "", Action action = Action{}) {
    auto&& ptr = ::gears::make_unique<typed_value<T>>(t);
    ptr->action(std::move(action));
    ptr->metavar = std::move(metavar);
    return std::unique_ptr<value_base>{std::move(ptr)};
}


template<typename T>
inline std::unique_ptr<value_base> constant(const T& t) {
    auto&& ptr = ::gears::make_unique<typed_value<T>>(store_const<T>{t});
    ptr->nargs = 0;
    return std::unique_ptr<value_base>{std::move(ptr)};
}


template<typename T, typename Action = store<T>>
inline std::unique_ptr<value_base> value(std::string metavar = "", Action action = Action{}) {
    auto&& ptr = ::gears::make_unique<typed_value<T>>(action);
    ptr->metavar = std::move(metavar);
    return std::unique_ptr<value_base>{std::move(ptr)};
}


template<typename T, typename Action>
inline std::unique_ptr<value_base> custom(Action action, std::string metavar = "") {
    auto&& ptr = ::gears::make_unique<typed_value<T>>(action);
    ptr->metavar = std::move(metavar);
    return std::unique_ptr<value_base>{std::move(ptr)};
}


template<typename Container, typename Action = store<typename Container::value_type>>
inline std::unique_ptr<value_base> list(size_t arguments, std::string metavar = "", Action action = Action{}) {
    static_assert(std::is_convertible<decltype(action("", "")), typename Container::value_type>::value,
                  "The action must return a type convertible to the container's value type");
    auto&& ptr = ::gears::make_unique<typed_value<Container>>(store_list<Container, Action>{action});
    ptr->nargs = arguments;
    ptr->metavar = std::move(metavar);
    return std::unique_ptr<value_base>{std::move(ptr)};
}
} // optparse
} // gears

// end of gears\optparse/value.hpp

// beginning of gears\enums/operators.hpp

// beginning of gears\enums/helpers.hpp

#include <type_traits>

namespace gears {
namespace enums {
namespace detail {
template<typename... Args>
struct are_enum : std::true_type {};

template<typename T>
struct are_enum<T> : std::is_enum<T> {};

template<typename T, typename U, typename... Args>
struct are_enum<T, U, Args...> : std::integral_constant<bool, std::is_enum<T>::value &&
                                                              std::is_enum<U>::value &&
                                                              are_enum<Args...>::value> {};

template<typename... Args>
using EnableIfEnum = typename std::enable_if<are_enum<typename std::remove_reference<Args>::type...>::value, bool>::type;
} // detail

template<typename Enum, typename Underlying = typename std::underlying_type<Enum>::type, detail::EnableIfEnum<Enum> = true>
constexpr Underlying to_underlying(Enum x) noexcept {
    return static_cast<Underlying>(x);
}

template<typename Enum, detail::EnableIfEnum<Enum> = true>
constexpr Enum activate_flags(const Enum& flag) noexcept {
    return flag;
}

template<typename Enum, detail::EnableIfEnum<Enum> = true>
constexpr Enum activate_flags(const Enum& first, const Enum& second) noexcept {
    return static_cast<Enum>(to_underlying(first) | to_underlying(second));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
constexpr Enum activate_flags(const Enum& first, const Enum& second, Enums&&... rest) noexcept {
    return static_cast<Enum>(to_underlying(activate_flags(first, second)) | to_underlying(activate_flags(rest...)));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
inline Enum& set_flags(Enum& flags, Enums&&... args) noexcept {
    return flags = activate_flags(flags, args...);
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
inline Enum& remove_flags(Enum& flags, Enums&&... args) noexcept {
    return flags = static_cast<Enum>(to_underlying(flags) & (~to_underlying(activate_flags(args...))));
}

template<typename Enum, typename... Enums, detail::EnableIfEnum<Enum, Enums...> = true>
constexpr bool has_flags(const Enum& flags, Enums&&... args) noexcept {
    return (to_underlying(flags) & to_underlying(activate_flags(args...))) == to_underlying(activate_flags(args...));
}
} // enums
} // gears

// end of gears\enums/helpers.hpp

namespace gears {
namespace enums {
namespace operators {
namespace detail {
template<typename Enum>
using Underlying = typename std::underlying_type<Enum>::type;
} // detail

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator~(const Enum& x) noexcept {
    return static_cast<Enum>(~ to_underlying(x));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator|(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) | to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator&(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) & to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr Enum operator^(const Enum& lhs, const Enum& rhs) noexcept {
    return static_cast<Enum>(to_underlying(lhs) ^ to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator|=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) | to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator&=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) & to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
inline Enum& operator^=(Enum& lhs, const Enum& rhs) noexcept {
    return lhs = static_cast<Enum>(to_underlying(lhs) ^ to_underlying(rhs));
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const Enum& lhs, const Enum& rhs) noexcept {
    return to_underlying(lhs) != to_underlying(rhs);
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const Enum& lhs, const detail::Underlying<Enum>& rhs) noexcept {
    return to_underlying(lhs) != rhs;
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator!=(const detail::Underlying<Enum>& lhs, const Enum& rhs) noexcept {
    return lhs != to_underlying(rhs);
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const Enum& lhs, const Enum& rhs) noexcept {
    return to_underlying(lhs) == to_underlying(rhs);
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const Enum& lhs, const detail::Underlying<Enum>& rhs) noexcept {
    return to_underlying(lhs) == rhs;
}

template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr bool operator==(const detail::Underlying<Enum>& lhs, const Enum& rhs) noexcept {
    return lhs == to_underlying(rhs);
}
} // operators
} // enums
} // gears

// end of gears\enums/operators.hpp

using namespace gears::enums::operators;

namespace gears {
namespace optparse {

enum class trait : char {
    none     = 0,        ///< Represents no traits being set.
    required = 1 << 0,   ///< The option is required to appear, or an error is thrown.
    hidden   = 1 << 1    ///< The option is suppressed from the default --help output.
};


struct option {
private:
    using value_type = std::unique_ptr<value_base>;
    value_type ptr;
    friend struct option_parser;
public:
    std::string name;   ///< The long name of the option, e.g. "help"
    std::string help;   ///< The description used for the help output
    trait flags;       ///< The trait flags to modify the option's behaviour
    char alias = '\0';  ///< The short name of the option, e.g. 'h'


    option(std::string name, std::string help = "", value_type value = constant(true), trait flags = trait::none):
        ptr(std::move(value)), name(std::move(name)), help(std::move(help)), flags(std::move(flags)) {}

    option(std::string name, char alias, std::string help = "", value_type value = constant(true), trait flags = trait::none):
        ptr(std::move(value)), name(std::move(name)), help(std::move(help)), flags(std::move(flags)), alias(alias) {}


    option(char alias, std::string help = "", value_type value = constant(true), trait flags = trait::none):
        ptr(std::move(value)), help(std::move(help)), flags(std::move(flags)), alias(alias) {}


    option(const option& other):
        ptr(other.ptr == nullptr ? nullptr : other.ptr->clone()),
        name(other.name), help(other.help), flags(other.flags), alias(other.alias) {}


    option& operator=(const option& other) {
        ptr = other.ptr == nullptr ? nullptr : other.ptr->clone();
        name = other.name;
        help = other.help;
        flags = other.flags;
        alias = other.alias;
        return *this;
    }


    option(option&&) = default;


    option& operator=(option&&) = default;


    ~option() = default;


    bool takes_value() const noexcept {
        return ptr != nullptr && ptr->nargs > 0;
    }


    size_t nargs() const noexcept {
        return takes_value() ? ptr->nargs : 0;
    }

    //@{

    bool is(char arg) const noexcept {
        return alias == arg;
    }

    bool is(const std::string& arg) const noexcept {
        return name == arg;
    }
    //@}


    template<typename T>
    const T& get() const {
        if(ptr == nullptr) {
            throw std::invalid_argument("option does not take value");
        }
        auto val = dynamic_cast<typed_value<T>*>(ptr.get());

        if(val == nullptr) {
            throw std::invalid_argument("invalid cast for option");
        }
        return val->get();
    }


    template<typename T>
    const T& get_or(const typename std::remove_reference<T>::type& def) const noexcept {
        if(ptr == nullptr) {
            return def;
        }

        auto val = dynamic_cast<typed_value<T>*>(ptr.get());
        return val == nullptr ? def : val->get_or(def);
    }


    bool is_active() const noexcept {
        return ptr != nullptr && ptr->is_active();
    }


    std::string metavar() const noexcept {
        return takes_value() ? ptr->metavar : "";
    }
};
} // optparse
} // gears

// end of gears\optparse/option.hpp

#include <set>
#include <vector>
#include <algorithm>

namespace gears {
namespace optparse {
namespace detail {
struct option_comparison {
    bool operator()(const option& lhs, const option& rhs) const {
        return lhs.name < rhs.name || (lhs.name == rhs.name && lhs.alias < rhs.alias);
    }
};
} // detail


struct option_set {
private:
    std::set<option, detail::option_comparison> cache;
    std::vector<option> options;
public:

    option_set(bool help = true) {
        if(help) {
            options.emplace_back("help", 'h', "shows this message and exits");
            cache.insert(options.back());
        }
    }


    option_set(std::initializer_list<option> l) {
        options.emplace_back("help", 'h', "shows this message and exits");
        cache.insert(options.back());

        for(auto&& i : l) {
            if(cache.count(i)) {
                continue;
            }

            cache.insert(i);
            options.push_back(i);
        }
    }


    template<typename... Args>
    void add(Args&&... args) {
        // emplace it to the vector
        options.emplace_back(std::forward<Args>(args)...);

        // check if it's in the cache and remove it if so
        auto&& it = cache.find(options.back());
        if(it != cache.end()) {
            options.pop_back();
        }

        cache.insert(options.back());
    }


    template<typename Argument>
    option_set& remove(const Argument& arg) {
        auto&& it = std::find_if(options.begin(), options.end(), [&arg](const option& opt) {
            return opt.is(arg);
        });

        if(it != options.end()) {
            cache.erase(*it);
            options.erase(it);
        }
        return *this;
    }


    template<typename T, typename Argument>
    const T& get(const Argument& arg) const {
        auto&& it = std::find_if(options.begin(), options.end(), [&arg](const option& opt) {
            return opt.is(arg);
        });

        if(it == options.end()) {
            throw std::invalid_argument("option not found");
        }

        return it->template get<T>();
    }


    template<typename T, typename Argument>
    const T& get_or(const Argument& arg, const typename std::remove_reference<T>::type& def) const noexcept {
        auto&& it = std::find_if(options.begin(), options.end(), [&arg](const option& opt) {
            return opt.is(arg);
        });

        if(it == options.end()) {
            return def;
        }

        return it->template get_or<T>(def);
    }


    template<typename Argument>
    bool is_active(const Argument& arg) const noexcept {
        auto&& it = std::find_if(options.begin(), options.end(), [&arg](const option& opt) {
            return opt.is(arg);
        });

        return it != options.end() && it->is_active();
    }


    size_t size() const noexcept {
        return options.size();
    }


    bool empty() const noexcept {
        return options.empty();
    }

    //@{

    auto begin() noexcept -> decltype(options.begin()) {
        return options.begin();
    }

    auto begin() const noexcept -> decltype(options.begin()) {
        return options.begin();
    }
    //@}

    //@{

    auto end() noexcept -> decltype(options.end()) {
        return options.end();
    }

    auto end() const noexcept -> decltype(options.end()) {
        return options.end();
    }
    //@}
};
} // optparse
} // gears

// end of gears\optparse/option_set.hpp

namespace gears {
namespace optparse {

struct subcommand {
    std::string name;                   ///< The name of the subcommand.
    std::string description;            ///< A brief paragraph giving an overview of the subcommand
    std::string help;                   ///< A sentence explaining the subcommand. Shown in the help message
    std::string usage = "[options..]";  ///< The usage message for the subcommand
    std::string epilogue;               ///< A paragraph usually printed after the help message
    option_set options;                 ///< The options the subcommand contains.


    subcommand() = default;


    subcommand(std::string name): name(std::move(name)) {}


    subcommand(std::string name, std::initializer_list<option> options): name(std::move(name)), options(std::move(options)) {}


    subcommand(std::string name, std::string help, std::initializer_list<option> options):
        name(std::move(name)), help(std::move(help)), options(std::move(options)) {}


    subcommand(std::string name, std::string help, option_set options):
        name(std::move(name)), help(std::move(help)), options(std::move(options)) {}


    subcommand(std::string name, option_set options): name(std::move(name)), options(std::move(options)) {}
};


struct arguments {
    const option_set& options;              ///< The current option_set
    std::vector<std::string> positional;    ///< The positional arguments
    std::string subcommand;                 ///< The active subcommand name
};
} // optparse
} // gears

// end of gears\optparse/subcommand.hpp

namespace gears {
namespace optparse {

struct formatter {
    size_t column;     ///< The maximum column before wrap around


    formatter(size_t column = 80): column(column) {}


    virtual ~formatter() = default;


    virtual std::string wrap(const std::string& str, size_t indent = 0) {
        if(str.empty()) {
            return str;
        }

        size_t c = column - 2;
        std::string result;
        std::string indentation(indent, ' ');

        if(str.size() + indent > c) {
            std::istringstream in(str);
            std::string word;
            if(in >> word) {
                result.append(word);
                size_t spaces = c - word.size() - indent;
                while(in >> word) {
                    if(spaces < word.size() + 1) {
                        result.append(1, '\n').append(indent, ' ').append(word);
                        spaces = c - word.size() - indent;
                    }
                    else {
                        result.append(1, ' ').append(word);
                        spaces -= word.size() + 1;
                    }
                }
            }
            result.push_back('\n');
        }
        else {
            result.append(str).push_back('\n');
        }

        return result;
    }


    virtual std::string usage(const std::string& name, const std::string& command, const std::string& str) {
        std::string result = "usage: " + name;
        if(!command.empty()) {
            result.push_back(' ');
            result += command;
        }
        result.push_back(' ');
        result += str;
        return wrap(result);
    }


    virtual std::string description(const std::string& str) {
        auto&& desc = wrap(str);
        if(!str.empty()) {
            desc.push_back('\n');
        }
        return desc;
    }


    virtual std::string epilogue(const std::string& str) {
        return wrap(str);
    }


    virtual std::string subcommands(const std::vector<subcommand>& subs) {
        if(subs.empty()) {
            return "";
        }

        std::string out = "subcommands\n";

        auto&& max = std::max_element(subs.begin(), subs.end(), [](const subcommand& lhs, const subcommand& rhs) {
            return lhs.name < rhs.name;
        });

        size_t indent = 20u;
        if(max != subs.end()) {
            indent += max->name.size();
        }

        for(auto&& i : subs) {
            out.append(4, ' ').append(i.name);
            size_t current = indent - 4 - i.name.size();
            out.append(current, ' ');
            if(!i.help.empty()) {
                out.append(wrap(i.help, current));
            }
            else {
                out.push_back('\n');
            }
        }

        return out;
    }


    virtual std::string options(const option_set& opts) {
        if(opts.empty()) {
            return "";
        }

        std::string result = "options:\n";

        auto&& max_name = std::max_element(opts.begin(), opts.end(), [](const option& lhs, const option& rhs) {
            return lhs.name < rhs.name;
        });

        auto&& max_meta = std::max_element(opts.begin(), opts.end(), [](const option& lhs, const option& rhs) {
            return lhs.metavar() < rhs.metavar();
        });

        size_t max_indent = 14u;

        if(max_name != opts.end()) {
            max_indent += max_name->name.size();
        }

        if(max_meta != opts.end()) {
            max_indent += 5u + max_meta->metavar().size();
        }

        for(auto&& opt : opts) {
            if((opt.flags & trait::hidden) == trait::hidden) {
                continue;
            }

            size_t indent = max_indent;
            const bool has_long_option  = !opt.name.empty();
            const bool has_short_option = opt.alias != '\0';
            const std::string metavar   = opt.metavar();
            const bool has_metavar      = !metavar.empty();

            if(has_long_option && has_short_option) {
                result.append(4, ' ').append(1, '-').append(1, opt.alias).append(", --").append(opt.name);
                indent -= 10u + opt.name.size();
            }
            else if(has_long_option) {
                result.append(4, ' ').append("    --").append(opt.name);
                indent -= 10u + opt.name.size();
            }
            else if(has_short_option) {
                result.append(4, ' ').append(1, '-').append(1, opt.alias);
                indent -= 6u;
            }

            if(has_metavar) {
                if(has_long_option) {
                    result.append("[=<").append(metavar).append(">]");
                    indent -= 5u + metavar.size();
                }
                else {
                    result.append(1, ' ').append(1, '<').append(metavar).append(1, '>');
                    indent -= 3u + metavar.size();
                }
            }

            result.append(indent, ' ');

            if(opt.help.empty()) {
                result.push_back('\n');
                continue;
            }

            result.append(wrap(opt.help, indent));
        }

        return result;
    }
};
} // optparse
} // gears

// end of gears\optparse/formatter.hpp

#include <iostream>
#include <cstdlib>

namespace gears {
namespace optparse {

struct option_parser {
private:
    std::vector<subcommand> subcommands;
    option_set options;
    std::unique_ptr<formatter> format = ::gears::make_unique<formatter>();
    option_set* active_options = &options;
    std::ptrdiff_t index = -1;

    bool is_option(const std::string& arg) const noexcept {
        return arg.size() >= 2 && arg.front() == '-';
    }

    template<typename ForwardIt>
    ForwardIt process_subcommand(ForwardIt begin, ForwardIt end) {
        if(begin == end) {
            return begin;
        }

        std::string arg = *begin;
        if(arg.empty()) {
            return begin;
        }

        auto&& it = std::find_if(subcommands.begin(), subcommands.end(), [&arg](const subcommand& sub) {
            return sub.name == arg;
        });

        if(it != subcommands.end()) {
            index = std::distance(subcommands.begin(), it);
            active_options = &(it->options);
            ++begin;
            return begin;
        }

        return begin;
    }

    template<typename ForwardIt>
    ForwardIt parse_long_option(std::string key, ForwardIt begin, ForwardIt end) {
        std::string value;
        auto&& pos = key.find('=');
        bool has_explicit_value = false;

        // check if it's --long=arg
        if(pos != std::string::npos) {
            value = key.substr(pos + 1);
            key = key.substr(0, pos);
            has_explicit_value = true;
        }

        // check if the argument exists
        auto&& it = std::find_if(active_options->begin(), active_options->end(), [&key](const option& opt) {
            return opt.is(key.substr(2));
        });

        if(it == active_options->end()) {
            throw unrecognised_option(program_name, key);
        }

        auto&& opt = *it;
        size_t argc = std::distance(begin, end);

        // option doesn't take a value but it was explicitly provided
        // so throw an error
        if(!opt.takes_value() && has_explicit_value) {
            throw option_takes_no_value(program_name, key);
        }

        if(opt.takes_value() && !has_explicit_value) {
            // check number of arguments left
            if(argc - 1 < opt.nargs()) {
                throw missing_required_value(program_name, key, opt.nargs());
            }

            // get the arguments needed
            for(size_t i = 0; i < opt.nargs(); ++i) {
                value += *(++begin);
                value.push_back('\n');
            }

            // remove extraneous newline
            if(!value.empty() && value.back() == '\n') {
                value.pop_back();
            }
        }

        // parse it
        if(opt.ptr != nullptr) {
            opt.ptr->parse(key, value);
        }

        return begin;
    }

    template<typename ForwardIt>
    ForwardIt parse_short_option(const std::string& arg, ForwardIt begin, ForwardIt end) {
        std::string value;
        // loop due to concatenation of short options
        for(size_t j = 1 ; j < arg.size(); ++j) {
            auto&& ch = arg[j];
            bool has_explicit_value = false;
            std::string key = "-";
            key.push_back(ch);

            // check for -o=stuff
            if(j + 2 < arg.size() && arg[j + 1] == '=') {
                value = arg.substr(j + 2);
                // set loop counter to end explicitly after
                // this loop
                j = arg.size();
                has_explicit_value = true;
            }

            auto&& it = std::find_if(active_options->begin(), active_options->end(), [&ch](const option& opt) {
                return opt.is(ch);
            });

            if(it == active_options->end()) {
                throw unrecognised_option(program_name, key);
            }

            auto&& opt = *it;
            size_t argc = std::distance(begin, end);

            if(has_explicit_value && !opt.takes_value()) {
                throw option_takes_no_value(program_name, key);
            }

            // check for -o stuff
            if(opt.takes_value() && !has_explicit_value) {
                // note that -ostuff doesn't work due to ambiguity.
                if(j + 1 != arg.size()) {
                    throw optparse::error(program_name, "short option \'" + key + "\' and value must not be combined", arg);
                }

                if(argc - 1 < opt.nargs()) {
                    throw missing_required_value(program_name, key, opt.nargs());
                }

                for(size_t i = 0; i < opt.nargs(); ++i) {
                    value += *(++begin);
                    value.push_back('\n');
                }

                // remove extraneous newline
                if(!value.empty() && value.back() == '\n') {
                    value.pop_back();
                }
            }

            if(opt.ptr != nullptr) {
                opt.ptr->parse(key, value);
            }
        }

        return begin;
    }

    template<typename It>
    arguments make_args(It begin, It end) const {
        if(index == -1) {
            return { *active_options, std::vector<std::string>(begin, end), "" };
        }
        auto&& sub = subcommands[index];
        return { *active_options, std::vector<std::string>(begin, end), sub.name };
    }
public:
    std::string description;              ///< A brief paragraph giving an overview of the program
    std::string epilogue;                 ///< A paragraph printed after the help message
    std::string program_name;             ///< The program name, if not provided it's argv[0]
    std::string usage = "[options...]";   ///< The usage string


    option_parser() = default;


    option_parser(option_set options): options(std::move(options)) {}


    option_parser(std::initializer_list<option> options): options(std::move(options)) {}


    template<typename... Args>
    void add(Args&&... args) {
        options.add(std::forward<Args>(args)...);
    }


    option_parser& add_subcommand(subcommand sub) {
        subcommands.push_back(std::move(sub));
        return *this;
    }


    template<typename Formatter>
    void help_formatter(const Formatter& form) {
        static_assert(std::is_base_of<formatter, Formatter>::value, "Must derive from formatter");
        format = ::gears::make_unique<Formatter>(form);
    }


    template<typename ForwardIt>
    arguments raw_parse(ForwardIt begin, ForwardIt end) {
        static_assert(std::is_constructible<std::string, decltype(*begin)>{},
                      "Iterator must return type convertible to std::string");

        if(begin == end) {
            return {*active_options, {}, ""};
        }

        // assign program name to argv[0] if it isn't provided
        if(program_name.empty()) {
            program_name = *begin++;
        }
        else {
            ++begin;
        }

        // check if argv[1] is a subcommand
        begin = process_subcommand(begin, end);

        // begin parsing command line arguments
        for(; begin != end; ++begin) {
            std::string arg = *begin;

            // -- is used to delimit options vs positional arguments
            if(arg == "--") {
                return make_args(++begin, end);
            }

            // check if it's an option
            if(is_option(arg)) {
                // check for long, i.e. --long
                if(arg[1] == '-') {
                    begin = parse_long_option(arg, begin, end);
                }
                else {
                    // short option, i.e. -s
                    begin = parse_short_option(arg, begin, end);
                }
            }
            else {
                // since it isn't an option, then the positional
                // arguments have begun so just stop parsing.
                break;
            }
        }

        return make_args(begin, end);
    }


    void notify() {
        for(auto&& opt : *active_options) {
            if((opt.flags & trait::required) == trait::required && !opt.is_active()) {
                throw missing_required_option(program_name, opt.name.empty() ? std::string("-").append(1, opt.alias)
                                                                             : "--" + opt.name);
            }
        }
    }


    template<typename ForwardIt>
    arguments parse(ForwardIt begin, ForwardIt end, std::ostream& out = std::cout, std::ostream& err = std::cerr) {
        try {
            auto&& args = raw_parse(begin, end);

            if(args.options.is_active("help")) {
                out << format_help();
                std::exit(EXIT_SUCCESS);
            }

            notify();

            return args;
        }
        catch(const optparse::error& e) {
            err << format_usage() << e.what() << '\n';
        }
        catch(const std::exception& e) {
            err << format_usage() << program_name << ": error: " << e.what() << '\n';
        }

        if(active_options->is_active("help")) {
            out << format_description()
                << format_subcommands()
                << format_options()
                << format_epilogue();
        }

        std::exit(EXIT_FAILURE);
    }


    std::string format_description() const noexcept {
        return format->description(index == -1 ? description : subcommands[index].description);
    }


    std::string format_epilogue() const noexcept {
        return format->epilogue(index == -1 ? epilogue : subcommands[index].epilogue);
    }


    std::string format_usage() const noexcept {
        return format->usage(program_name,
                             index == -1 ? "" : subcommands[index].name,
                             index == -1 ? usage : subcommands[index].usage);
    }


    std::string format_subcommands() const noexcept {
        return format->subcommands(subcommands);
    }


    std::string format_options() const noexcept {
        return format->options(*active_options);
    }


    std::string format_help() const noexcept {
        std::string result = format_usage();
        result.append(1, '\n').append(format_description());

        if(active_options == &options) {
            result.append(format_subcommands());

            if(!subcommands.empty()) {
                result.push_back('\n');
            }
        }

        result.append(format_options()).append(format_epilogue());
        return result;
    }


    void error(const std::string& message, std::ostream& err = std::cerr) {
        err << format_usage();
        err << format->wrap(program_name + ": error: " + message);
        std::exit(EXIT_FAILURE);
    }
};
} // optparse
} // gears

// end of gears\optparse/option_parser.hpp



// end of gears\optparse.hpp

#endif // GEARS_SINGLE_INCLUDE_OPTPARSE_HPP
