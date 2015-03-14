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

#ifndef HYPEST_CACHE_HPP
#define HYPEST_CACHE_HPP

#include <string>
#include <vector>

namespace hypest {
struct cache_entry {
    std::string url;     // the challonge URL it points to
    int rating_period;   // denotes the rating period it belongs to (starts at 1)
    int id;              // denotes the entry ID (starts at 1)
};

// this type shall be sorted when loaded, making it safe for binary_search
struct rank_cache {
    std::vector<cache_entry> entries;
    int current_rating_period = 1;

    auto begin() {
        return entries.begin();
    }

    auto end() {
        return entries.end();
    }

    auto begin() const {
        return entries.begin();
    }

    auto end() const {
        return entries.end();
    }

    auto size() const {
        return entries.size();
    }

    void add(const std::string& url);
    bool in(const std::string& url);
};

inline bool operator<(const cache_entry& lhs, const cache_entry& rhs) {
    return lhs.url < rhs.url;
}

rank_cache get_cache();
void update_cache(const rank_cache& cache);
} // hypest

#endif // HYPEST_CACHE_HPP
