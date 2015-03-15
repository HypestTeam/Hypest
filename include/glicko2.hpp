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

#ifndef HYPEST_GLICKO2_HPP
#define HYPEST_GLICKO2_HPP

#include <vector>
#include <algorithm>
#include <cmath>

namespace hypest {
constexpr double pow2(double x) noexcept {
    return x * x;
}

// http://www.glicko.net/glicko/glicko2.pdf
class glicko {
public:
    static constexpr double tau = 0.5;
    static constexpr double pi = 3.141592653589793238463;
    static constexpr double scale = 173.7178;
    static constexpr double epsilon = 0.000001;
    glicko() noexcept = default;
    glicko(double r, double rdev = 350, double v = 0.06): vol(v) {
        rating(r);
        rd(rdev);
    }

    template<typename T = double>
    T rating() const noexcept {
        return static_cast<T>(scale * scaled_rating) + 1500;
    }

    void rating(double r) noexcept {
        scaled_rating = (r - 1500) / scale;
    }

    void rd(double arg) noexcept {
        scaled_rd = arg / scale;
    }

    double rd() const noexcept {
        return scale * scaled_rd;
    }

    double volatility() const noexcept {
        return vol;
    }

    void volatility(double v) noexcept {
        vol = v;
    }

    void has_participated() noexcept {
        participating = true;
    }

    bool is_participating() const noexcept {
        return participating;
    }

    template<typename Matches>
    void update(const Matches& matches) {
        if(not participating) {
            scaled_rd = std::sqrt(pow2(scaled_rd) + pow2(vol));
            return;
        }
        double v = calculate_v(matches);
        double scores = score_sum(matches);
        double delta = scores * v;
        double new_vol = new_volatility(delta, v);
        double pre_rating_period = std::sqrt(pow2(scaled_rd) + pow2(new_vol));
        double new_rd = 1 / std::sqrt(1/pow2(pre_rating_period) + 1/v);
        double new_rating = scaled_rating + pow2(new_rd) * scores;
        // actually update
        scaled_rating = new_rating;
        scaled_rd = new_rd;
        vol = new_vol;
    }
private:
    double scaled_rating = 0.0;
    double scaled_rd = 350 / scale;
    double vol = 0.06;
    bool participating = false;

    // the g(o) function in Glicko-2
    double g(double op_rd) const noexcept {
        return 1 / std::sqrt(1 + (3 * pow2(op_rd))/pow2(pi));
    }

    template<typename Matches>
    bool has_played(const Matches& matches) const noexcept {
        return std::find_if(matches.begin(), matches.end(), [this](const auto& m) {
            return m.p1 == this;
        }) != matches.end();
    }

    // the E(micro, micro_j, o_j) function in Glicko-2
    // g_o is the result of g(opponent_vol) which could be pre-computed
    double E(double opponent_rating, double g_o) const noexcept {
        return 1 / (1 + std::exp(-g_o * (scaled_rating - opponent_rating)));
    }

    // step 3 to calculate quantity v
    template<typename Matches>
    double calculate_v(const Matches& matches) const noexcept {
        double sum = 0.0;
        for(auto&& match : matches) {
            if(match.p1 != this) {
                continue;
            }
            auto* opponent = match.p2;
            double g_o = g(opponent->scaled_rd);
            double e = E(opponent->scaled_rating, g_o);
            sum += pow2(g_o) * e * (1 - e);
        }
        return 1 / sum;
    }

    // part of step 4 and step 7 is to get the sum of scores
    template<typename Matches>
    double score_sum(const Matches& matches) const noexcept {
        double sum = 0.0;
        for(auto&& match : matches) {
            if(match.p1 != this) {
                continue;
            }
            auto* opponent = match.p2;
            double g_o = g(opponent->scaled_rd);
            double e = E(opponent->scaled_rating, g_o);
            sum += g_o * (match.score() - e);
        }
        return sum;
    }

    // the f(x) function in Glicko-2
    double f(double x, double delta, double v, double a) const noexcept {
        auto e = std::exp(x);
        double top = e * (pow2(delta) - pow2(scaled_rd) - v - e);
        double bottom = (pow2(scaled_rd) + v + e);
        bottom = 2 * pow2(bottom);
        return (top / bottom) - ((x - a) / pow2(tau));
    }

    // step 4 to calculate the new volatility
    double new_volatility(double delta, double v) const noexcept {
        double a = std::log(pow2(vol)); // ln(vol^2)
        double A = a;
        double B;
        if(pow2(delta) > (pow2(scaled_rd) + v)) {
            B = std::log(pow2(delta) - pow2(scaled_rd) - v);
        }
        else {
            int k = 1;
            for(; ;) {
                if(f(a - k * tau, delta, v, a) < 0.) {
                    ++k;
                    continue;
                }
                break;
            }
            B = a - k * tau;
        }

        double f_A = f(A, delta, v, a);
        double f_B = f(B, delta, v, a);
        while(std::abs(B - A) > epsilon) {
            double C = A + ((A - B) * f_A) / (f_B - f_A);
            double f_C = f(C, delta, v, a);
            if(f_C * f_B < 0.0) {
                A = B;
                f_A = f_B;
            }
            else {
                f_A /= 2;
            }
            B = C;
            f_B = f_C;
        }

        return std::exp(A / 2);
    }
};

struct match {
    enum : int {
        win, loss, tie
    };

    glicko* p1;
    glicko* p2;
    int result;

    match(glicko* p1, glicko* p2, int result) noexcept: p1(p1), p2(p2), result(result) {
        p1->has_participated();
        p2->has_participated();
    }

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

#endif // HYPEST_GLICKO2_HPP
