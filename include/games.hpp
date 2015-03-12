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

#ifndef HYPEST_GAMES_HPP
#define HYPEST_GAMES_HPP

#include <string>
#include <vector>

namespace hypest {
struct game {
    std::string name;
    std::string filename;
    int id;
};

inline std::vector<game> games() noexcept {
    static std::vector<game> result = {
        { "Super Smash Bros. for 3DS",   "ssb3ds.json",   16869 },
        { "Super Smash Bros. for Wii U", "ssbwiiu.json",  20988 },
        { "Super Smash Bros. Melee",     "ssbm.json",     394   },
        { "Super Smash Bros.",           "ssb64.json",    392   },
        { "Project M",                   "projectm.json", 597   },
        { "Super Smash Flash 2",         "ssf2.json",     1106  }
    };
    return result;
}
} // hypest

#endif // HYPEST_GAMES_HPP
