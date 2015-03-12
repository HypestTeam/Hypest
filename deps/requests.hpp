// The MIT License (MIT)

// Copyright (c) 2015 Danny "Rapptz" Y.

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
// Generated 2015-03-08 03:10:36.061000 UTC
// This header was generated with requests  (revision 95d2348)
// https://github.com/Rapptz/requests

#ifndef REQUESTS_SINGLE_INCLUDE_HPP
#define REQUESTS_SINGLE_INCLUDE_HPP

// beginning of requests\auth.hpp

// beginning of requests\detail\init.hpp

#include <curl/curl.h>
#include <atomic>
#include <stdexcept>

namespace requests {
namespace detail {
struct init {
private:
    static std::atomic<int> counter;
public:
    init() {
        if(++counter == 1) {
            auto error = curl_global_init(CURL_GLOBAL_ALL);
            if(error) {
                throw std::runtime_error("failure to initialise libcurl");
            }
        }
    }

    init(const init&) = delete;
    init& operator=(const init&) = delete;
    init(init&&) = delete;
    init& operator=(init&&) = delete;

    ~init() {
        if(--counter == 0) {
            curl_global_cleanup();
        }
    }
};

std::atomic<int> init::counter{0};
static init libcurl_init;
} // detail
} // requests

// end of requests\detail\init.hpp

// beginning of requests\error.hpp

#include <stdexcept>

namespace requests {
struct libcurl_error : public std::runtime_error {
    CURLcode code;
    libcurl_error(CURLcode code): std::runtime_error{curl_easy_strerror(code)}, code(code) {}
    libcurl_error(const std::string& str, CURLcode code): std::runtime_error{str + ": " + curl_easy_strerror(code)}, code(code) {}
};

template<typename Option, typename Parameter>
inline void safe_setopt(CURL* handle, Option op, Parameter param) {
    auto res = curl_easy_setopt(handle, op, param);
    if(res == CURLE_OK || res == CURLE_NOT_BUILT_IN || CURLE_UNKNOWN_OPTION) {
        return;
    }
    throw libcurl_error(res);
}
} // requests

// end of requests\error.hpp

#include <string>

namespace requests {
template<long bitflag>
struct http_auth {
    std::string username;
    std::string password;

    void apply(CURL* handle) const {
        safe_setopt(handle, CURLOPT_USERNAME, username.c_str());
        safe_setopt(handle, CURLOPT_PASSWORD, password.c_str());
        safe_setopt(handle, CURLOPT_HTTPAUTH, bitflag);
    }
};

inline http_auth<CURLAUTH_BASIC> auth(std::string username, std::string password) {
    return http_auth<CURLAUTH_BASIC>{ std::move(username), std::move(password) };
}
} // requests

// end of requests\auth.hpp

// beginning of requests\header_list.hpp

// beginning of requests\type_traits.hpp

// beginning of requests\detail\string_traits.hpp

#include <string>

namespace requests {
namespace detail {
template<typename T>
struct string_traits {};

template<typename... Rest>
struct string_traits<std::basic_string<char, Rest...>> {
    using value_type = std::basic_string<char, Rest...>;

    static const char* c_str(const value_type& str) noexcept {
        return str.c_str();
    }

    static auto size(const value_type& str) noexcept -> decltype(str.size()) {
        return str.size();
    }
};

template<std::string::size_type N>
struct string_traits<char[N]> {
    static const char* c_str(const char (&arr)[N]) noexcept {
        return arr;
    }

    static auto size(const char (&)[N]) noexcept -> decltype(N - 1) {
        return N - 1;
    }
};

template<>
struct string_traits<const char*> {
    static const char* c_str(const char* str) noexcept {
        return str;
    }

    static auto size(const char* str) noexcept -> decltype(std::char_traits<char>::length(str)) {
        return std::char_traits<char>::length(str);
    }
};

template<>
struct string_traits<char*> {
    static const char* c_str(char* str) noexcept {
        return str;
    }

    static auto size(char* str) noexcept -> decltype(std::char_traits<char>::length(str)) {
        return std::char_traits<char>::length(str);
    }
};

struct is_string {
    template<typename T>
    static auto test(int) -> decltype(&string_traits<T>::c_str, std::true_type{});
    template<typename...>
    static std::false_type test(...);
};
} // detail

template<typename T>
struct is_string : decltype(detail::is_string::test<T>(0)) {};
} // requests

// end of requests\detail\string_traits.hpp

#include <type_traits>

namespace requests {
namespace detail {
struct is_key_value_container {
    template<typename T,
             typename B = decltype(std::declval<T>().begin()),
             typename E = decltype(std::declval<T>().end()),
             typename V = decltype(*std::declval<B>()),
             typename F = decltype(std::declval<V>().first),
             typename S = decltype(std::declval<V>().second),
             bool     R = ::requests::is_string<F>::value && ::requests::is_string<S>::value>
    static std::integral_constant<bool, R> test(int);
    template<typename...>
    static std::false_type test(...);
};

struct is_authenticator {
    template<typename T,
             typename A = decltype(std::declval<T>().apply(std::declval<CURL*>()))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};
} // detail

template<typename T>
struct is_key_value_container : decltype(detail::is_key_value_container::test<T>(0)) {};

template<typename T>
struct is_authenticator : decltype(detail::is_authenticator::test<T>(0)) {};

template<typename T>
using enable_if_t = typename std::enable_if<T::value, int>::type;

template<typename T>
using disable_if_t = typename std::enable_if<!T::value, int>::type;
} // requests

// end of requests\type_traits.hpp

namespace requests {
struct header_list {
public:
    header_list() = default;

    header_list(header_list&& other) noexcept: list(other.list) {
        other.list = nullptr;
    }

    template<typename StringPair>
    header_list(const StringPair& pairs) {
        for(auto&& pair : pairs) {
            append(pair.first, pair.second);
        }
    }

    header_list(const header_list& other) {
        copy(other.list);
    }

    header_list& operator=(const header_list& other) {
        clear();
        copy(other.list);
        return *this;
    }

    header_list& operator=(header_list&& other) noexcept {
        clear();
        list = other.list;
        other.list = nullptr;
        return *this;
    }

    ~header_list() {
        clear();
    }

    template<typename String>
    void append(const String& header) {
        using trait_type = detail::string_traits<String>;
        list = curl_slist_append(list, trait_type::c_str(header));
        if(list == nullptr) {
            throw std::runtime_error("could not append header");
        }
    }

    template<typename LeftString, typename RightString>
    void append(const LeftString& key, const RightString& value) {
        auto result = key + std::string(": ") + value;
        list = curl_slist_append(list, result.c_str());
        if(list == nullptr) {
            throw std::runtime_error("could not append key/value pair");
        }
    }

    void clear() noexcept {
        if(list != nullptr) {
            curl_slist_free_all(list);
            list = nullptr;
        }
    }

    curl_slist* data() const noexcept {
        return list;
    }
private:
    void copy(curl_slist* other) {
        for(auto ptr = other; ptr != nullptr; ptr = ptr->next) {
            append(ptr->data);
        }
    }
    curl_slist* list = nullptr;
};
} // requests

// end of requests\header_list.hpp

// beginning of requests\request.hpp

// beginning of requests\detail\easy.hpp

#include <curl/curl.h>
#include <memory>
#include <stdexcept>

namespace requests {
namespace detail {
struct easy_handle_deleter {
    void operator()(CURL* handle) const noexcept {
        if(handle != nullptr) {
            curl_easy_cleanup(handle);
        }
    }
};

using curl_ptr = std::unique_ptr<CURL, easy_handle_deleter>;
} // detail

inline detail::curl_ptr easy_handle() noexcept {
    return detail::curl_ptr{ curl_easy_init() };
}
} // requests

// end of requests\detail\easy.hpp

// beginning of requests\response.hpp

#include <string>
#include <unordered_map>
#include <cstddef>
#include <iostream>

namespace requests {
struct response {
    std::unordered_map<std::string, std::string> headers;
    std::string text;
    long status_code;

    static size_t write(void* data, size_t size, size_t count, void* userdata) {
        auto ptr = static_cast<response*>(userdata);
        ptr->text.append(static_cast<char*>(data), size * count);
        return size * count;
    }

    static size_t header(char* data, size_t size, size_t count, void* userdata) {
        auto ptr = static_cast<response*>(userdata);
        std::string header(data);
        // if this is an HTTP header then just exit early
        if(header.find("HTTP") == 0) {
            return size * count;
        }

        // 'trim' the end
        while(header.back() == '\n' || header.back() == '\t' || header.back() == ' ' || header.back() == '\r') {
            header.pop_back();
        }

        // separate the header to key/value pairs
        auto pos = header.find(':');
        if(pos != std::string::npos) {
            ptr->headers.emplace(header.substr(0, pos), header.substr(pos + 2));
        }
        return size * count;
    }
};
} // requests

// end of requests\response.hpp

#include <memory>
#include <functional>
#include <future>

namespace requests {
struct request {
public:
    template<typename String>
    request(const String& url): url(url), handle(easy_handle()) {
        if(handle == nullptr) {
            throw std::runtime_error("unable to create a libcURL easy handle");
        }

        // prepare the URL
        setopt(CURLOPT_URL, this->url.c_str());
        setopt(CURLOPT_WRITEFUNCTION, &response::write);
        setopt(CURLOPT_WRITEDATA, &res);
        setopt(CURLOPT_HEADERFUNCTION, &response::header);
        setopt(CURLOPT_HEADERDATA, &res);
        setopt(CURLOPT_SSL_VERIFYPEER, 0L);
        setopt(CURLOPT_SSL_VERIFYHOST, 0L);
        setopt(CURLOPT_USERAGENT, "requests for cpp");
    }

    template<typename String>
    request& user_agent(const String& str) {
        setopt(CURLOPT_USERAGENT, detail::string_traits<String>::c_str(str));
        return *this;
    }

    request& allow_redirect(bool allow = true) {
        setopt(CURLOPT_FOLLOWLOCATION, static_cast<long>(allow));
        return *this;
    }

    request& headers(header_list list) {
        slist = std::move(list);
        return *this;
    }

    template<typename Auth>
    request& authenticate(const Auth& auth) {
        auth.apply(handle.get());
        return *this;
    }

    template<typename Param>
    request& parameters(const Param& param) {
        auto temp = url;
        static_assert(is_key_value_container<Param>::value,
                      "Type passed must be a container containing an std::pair of strings.");
        url.push_back('?');
        auto&& it  = param.begin();
        auto&& end = param.end();
        if(it != end) {
           unescape_string(url, *it, temp);
           ++it;
        }

        while(it != end) {
            url.push_back('&');
            unescape_string(url, *it, temp);
            ++it;
        }

        // set the new URL
        setopt(CURLOPT_URL, url.c_str());
        return *this;
    }

    template<typename String, disable_if_t<is_key_value_container<String>> = 0>
    request& data(const String& str) {
        setopt(CURLOPT_POSTFIELDSIZE, detail::string_traits<String>::size(str));
        setopt(CURLOPT_COPYPOSTFIELDS, detail::string_traits<String>::c_str(str));
        slist.append("Content-Type:");
        return *this;
    }

    template<typename PairList, enable_if_t<is_key_value_container<PairList>> = 0>
    request& data(const PairList& pairs) {
        std::string result;
        std::string temp  = ""; // unused
        bool first = true;
        for(auto&& pair : pairs) {
            if(not first) {
                result.push_back('&');
            }
            unescape_string(result, pair, temp);
            first = false;
        }

        setopt(CURLOPT_POSTFIELDSIZE, result.size());
        setopt(CURLOPT_COPYPOSTFIELDS, result.c_str());
        return *this;
    }

    template<typename String>
    response send(const String& method) {
        setopt(CURLOPT_HTTPHEADER, slist.data());

        using comp = std::char_traits<char>;
        using trait_type = detail::string_traits<String>;
        const auto size = trait_type::size(method);
        auto cstr = trait_type::c_str(method);

        if(comp::compare(cstr, "GET", size) == 0) {
            setopt(CURLOPT_HTTPGET, 1L);
        }
        else if(comp::compare(cstr, "HEAD", size) == 0) {
            setopt(CURLOPT_HTTPGET, 1L);
            setopt(CURLOPT_NOBODY, 1L);
        }
        else if(comp::compare(cstr, "POST", size) == 0) {
            setopt(CURLOPT_POST, 1L);
        }
        else if(comp::compare(cstr, "PUT", size) == 0) {
            setopt(CURLOPT_UPLOAD, 1L);
        }
        else {
            setopt(CURLOPT_CUSTOMREQUEST, cstr);
        }

        auto error = curl_easy_perform(handle.get());
        if(error) {
            throw libcurl_error("failure to do get request", error);
        }
        fill_response();
        return res;
    }

    template<typename String>
    std::future<response> async_send(const String& method) {
        return std::async(std::launch::async, &request::send<String>, this, std::cref(method));
    }
private:
    template<typename Option, typename Parameter>
    void setopt(Option opt, Parameter param) {
        auto res = curl_easy_setopt(handle.get(), opt, param);
        if(res == CURLE_OK || res == CURLE_NOT_BUILT_IN || CURLE_UNKNOWN_OPTION) {
            return; // these errors are fine
        }
        // the rest aren't so throw an error
        throw libcurl_error(res);
    }

    void fill_response() {
        auto error = curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &res.status_code);
        if(error) {
            throw libcurl_error(error);
        }
    }

    template<typename Pair>
    void unescape_string(std::string& str, const Pair& pair, const std::string& temp) {
        using trait_type = detail::string_traits<decltype(pair.first)>;
        str.append(pair.first);
        str.push_back('=');
        auto res = curl_easy_escape(handle.get(), trait_type::c_str(pair.second), trait_type::size(pair.second));
        if(res == nullptr) {
            str = temp;
            throw std::runtime_error(std::string("unable to escape string: ") + pair.second);
        }
        str.append(res);
        curl_free(res);
    }

    response res;
    std::string url;
    detail::curl_ptr handle;
    header_list slist;
};

template<typename String>
inline response get(const String& url) {
    request req(url);
    return req.send("GET");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response get(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("GET");
}

template<typename String, typename Parameters, disable_if_t<is_authenticator<Parameters>> = 0>
inline response get(const String& url, const Parameters& param) {
    request req(url);
    req.parameters(param);
    return req.send("GET");
}

template<typename String, typename Auth, typename Parameters>
inline response get(const String& url, const Auth& auth, const Parameters& param) {
    request req(url);
    req.authenticate(auth);
    req.parameters(param);
    return req.send("GET");
}

template<typename... Args>
inline std::future<response> async_get(const Args&... args) {
    return std::async(std::launch::async, get<Args...>, std::cref(args)...);
}

template<typename String>
inline response head(const String& url) {
    request req(url);
    return req.send("HEAD");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response head(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("HEAD");
}

template<typename String, typename Parameters, disable_if_t<is_authenticator<Parameters>> = 0>
inline response head(const String& url, const Parameters& param) {
    request req(url);
    req.parameters(param);
    return req.send("HEAD");
}

template<typename String, typename Auth, typename Parameters>
inline response head(const String& url, const Auth& auth, const Parameters& param) {
    request req(url);
    req.authenticate(auth);
    req.parameters(param);
    return req.send("HEAD");
}

template<typename... Args>
inline std::future<response> async_head(const Args&... args) {
    return std::async(std::launch::async, head<Args...>, std::cref(args)...);
}

template<typename String>
inline response post(const String& url) {
    request req(url);
    return req.send("POST");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response post(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("POST");
}

template<typename String, typename Data, disable_if_t<is_authenticator<Data>> = 0>
inline response post(const String& url, const Data& data) {
    request req(url);
    req.data(data);
    return req.send("POST");
}

template<typename String, typename Auth, typename Data>
inline response post(const String& url, const Auth& auth, const Data& data) {
    request req(url);
    req.authenticate(auth);
    req.data(data);
    return req.send("POST");
}

template<typename... Args>
inline std::future<response> async_post(const Args&... args) {
    return std::async(std::launch::async, post<Args...>, std::cref(args)...);
}

template<typename String>
inline response put(const String& url) {
    request req(url);
    return req.send("PUT");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response put(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("PUT");
}

template<typename String, typename Data, disable_if_t<is_authenticator<Data>> = 0>
inline response put(const String& url, const Data& data) {
    request req(url);
    req.data(data);
    return req.send("PUT");
}

template<typename String, typename Auth, typename Data>
inline response put(const String& url, const Auth& auth, const Data& data) {
    request req(url);
    req.authenticate(auth);
    req.data(data);
    return req.send("PUT");
}

template<typename... Args>
inline std::future<response> async_put(const Args&... args) {
    return std::async(std::launch::async, put<Args...>, std::cref(args)...);
}

template<typename String>
inline response delete_(const String& url) {
    request req(url);
    return req.send("DELETE");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response delete_(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("DELETE");
}

template<typename String, typename Parameters, disable_if_t<is_authenticator<Parameters>> = 0>
inline response delete_(const String& url, const Parameters& param) {
    request req(url);
    req.parameters(param);
    return req.send("DELETE");
}

template<typename String, typename Auth, typename Parameters>
inline response delete_(const String& url, const Auth& auth, const Parameters& param) {
    request req(url);
    req.authenticate(auth);
    req.parameters(param);
    return req.send("DELETE");
}

template<typename... Args>
inline std::future<response> async_delete(const Args&... args) {
    return std::async(std::launch::async, delete_<Args...>, std::cref(args)...);
}

template<typename String>
inline response patch(const String& url) {
    request req(url);
    return req.send("PATCH");
}

template<typename String, typename Auth, enable_if_t<is_authenticator<Auth>> = 0>
inline response patch(const String& url, const Auth& auth) {
    request req(url);
    req.authenticate(auth);
    return req.send("PATCH");
}

template<typename String, typename Data, disable_if_t<is_authenticator<Data>> = 0>
inline response patch(const String& url, const Data& data) {
    request req(url);
    req.data(data);
    return req.send("PATCH");
}

template<typename String, typename Auth, typename Data>
inline response patch(const String& url, const Auth& auth, const Data& data) {
    request req(url);
    req.authenticate(auth);
    req.data(data);
    return req.send("PATCH");
}

template<typename... Args>
inline std::future<response> async_patch(const Args&... args) {
    return std::async(std::launch::async, patch<Args...>, std::cref(args)...);
}
} // requests

// end of requests\request.hpp

#endif // REQUESTS_SINGLE_INCLUDE_HPP
