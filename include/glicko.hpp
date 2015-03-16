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

#ifndef HYPEST_GLICKO_HPP
#define HYPEST_GLICKO_HPP

#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

namespace hypest {
constexpr double pow2(double x) noexcept {
    return x * x;
}

template<typename T, typename U>
constexpr auto min(const T& a, const U& b) {
    return (b < a) ? b : a;
}

template<typename T, typename U>
constexpr auto max(const T& a, const U& b) {
    return (b < a) ? a : b;
}

static constexpr double max_rd = 350.0;
static constexpr double min_rd = 35.0;

// http://www.glicko.net/glicko/glicko.pdf
class glicko {
public:

    // this is the decay constant that dictates how many rating
    // periods it will take to go from min_rd to max_rd.
    // At the moment this constant is derived from 18 rating periods.
    // The formula to do this is max_rd = sqrt(min_rd^2 + decay^2 * t)
    // where t is the number of rating periods missed (in this case 18)
    static constexpr double decay = 82.0822757969100;

    glicko() noexcept = default;
    glicko(double r, double rdev = max_rd): r(r), rdev(rdev) {}

    template<typename T = double>
    T rating() const noexcept {
        return r;
    }

    void rating(double r) noexcept {
        this->r = r;
    }

    void rd(double arg) noexcept {
        rdev = arg;
    }

    double rd() const noexcept {
        return rdev;
    }

    void has_participated() noexcept {
        participating = true;
        t = 1;
    }

    bool is_participating() const noexcept {
        return participating;
    }

    int rating_periods_passed() const noexcept {
        return t;
    }

    void rating_periods_passed(int arg) {
        t = arg;
    }

    template<typename Matches>
    void update(const Matches& matches) {
        int period = participating ? t : t + 1;
        rdev = min(std::sqrt(pow2(rdev) + pow2(decay) * period), max_rd);
        rdev = max(min_rd, rdev);
        if(not participating) {
            t = period;
            return;
        }
        auto&& p = calculate(matches);
        double bottom = 1 / pow2(rdev) + 1 / p.first;
        double new_rating = r + (q / bottom) * p.second;
        double new_rdev = std::sqrt(1 / bottom);

        // actually update
        r = new_rating;
        rdev = max(min_rd, new_rdev);
    }
private:
    static constexpr double q =  0.005756462732485114210;
    static constexpr double pi = 3.141592653589793238463;

    double r = 1500.0;
    double rdev = max_rd;
    int t = 1;
    bool participating = false;

    glicko(double r, double rdev, int t, bool p): r(r), rdev(rdev), t(t), participating(p) {}

    double g(double rdj) const noexcept {
        return 1.0 / std::sqrt(1 + (3 * pow2(q) * pow2(rdj)) / pow2(pi));
    }

    double E(double rj, double rdj) const noexcept {
        return 1.0 / (1 + std::pow(10, (-g(rdj) * (r - rj)) / 400));
    }

    // calculates the value multiplied by the new rating and d^2 for Glicko-1 in a single pass
    template<typename Matches>
    std::pair<double, double> calculate(const Matches& matches) const noexcept {
        double v = 0.0;
        double d = 0.0;
        for(auto&& match : matches) {
            double e = E(match.opponent.r, match.opponent.rdev);
            double gr = g(match.opponent.rdev);
            v += gr * (match.score() - e);
            d += pow2(gr) * e * (1 - e);
        }
        d *= pow2(q);
        return { 1 / d, v };
    }
};

struct match {
    enum : int {
        win, loss, tie
    };

    glicko opponent;
    int result;

    match(glicko opponent, int result) noexcept: opponent(opponent), result(result) {}

    double score() const noexcept {
        switch(result) {
        case match::win:
            return 1.0;
        case match::loss:
            return 0.0;
        case match::tie:
            return 0.5;
        default:
            return 0.0;
        }
    }
};
} // hypest

#endif // HYPEST_GLICKO_HPP
