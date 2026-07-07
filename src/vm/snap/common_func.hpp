#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include "stream_macros.hpp"
namespace Hazel{
static inline std::int64_t __attribute__((hot)) is_in_range_with_step(std::int64_t n, std::int64_t low, std::int64_t high, std::int64_t step) {
    if (step == 0) {
        return (low == high) && (n == low);
    }
    step = std::abs(step); // Ensure step is positive
    if (n < low || n > high) {
        return false;
    }
    return (n - low) % step == 0;
}

static inline std::int64_t __attribute__((hot)) fixed_point_floor(std::int64_t a){
    const std::int64_t value = (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
    if (value == a) {
        return value;
    }
    return (a < 0) ? value - FIXED_POINT_FLOAT_SCALING_FACTOR : value;
}

static inline std::int64_t __attribute__((hot)) fixed_point_ceil(std::int64_t a){
    const std::int64_t value = (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
    if (value == a) {
        return value;
    }
    return (a > 0) ? value + FIXED_POINT_FLOAT_SCALING_FACTOR : value;
}

static inline std::int64_t __attribute__((hot)) fixed_point_integral_part(std::int64_t a){
    return (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
}

static inline std::int64_t __attribute__((hot)) fixed_point_fractional_part(std::int64_t a){
    return a - (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
}

static inline std::int64_t __attribute__((hot)) fixed_point_roundnearest(std::int64_t a){
    const std::int64_t integral = (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
    const std::int64_t fractional = a - integral;
    if (fractional >= FIXED_POINT_FLOAT_SCALING_FACTOR / 2) {
        return integral + FIXED_POINT_FLOAT_SCALING_FACTOR;
    }
    else if(fractional <= -FIXED_POINT_FLOAT_SCALING_FACTOR / 2) {
        return integral - FIXED_POINT_FLOAT_SCALING_FACTOR;
    }
    return integral;
}
static inline std::int64_t __attribute__((hot)) fixed_point_roundeven(std::int64_t a){
    const std::int64_t integral = (a/FIXED_POINT_FLOAT_SCALING_FACTOR)*FIXED_POINT_FLOAT_SCALING_FACTOR;
    const std::int64_t fractional = a - integral;
    if (fractional > FIXED_POINT_FLOAT_SCALING_FACTOR / 2) {
        return integral + FIXED_POINT_FLOAT_SCALING_FACTOR;
    }
    else if(fractional < -FIXED_POINT_FLOAT_SCALING_FACTOR / 2) {
        return integral - FIXED_POINT_FLOAT_SCALING_FACTOR;
    }
    else if(fractional == FIXED_POINT_FLOAT_SCALING_FACTOR / 2 || fractional == -FIXED_POINT_FLOAT_SCALING_FACTOR / 2){
        return (integral/FIXED_POINT_FLOAT_SCALING_FACTOR) % 2 == 0 ? integral : integral + (fractional > 0 ? FIXED_POINT_FLOAT_SCALING_FACTOR : -FIXED_POINT_FLOAT_SCALING_FACTOR);
    }
    return integral;
}

// Integer sqrt (floor) via Newton's method, operating in 128-bit to avoid overflow on the a*SCALE intermediate.
static inline __int128 __isqrt128(__int128 n) {
    if (n < 2) return n; // handles n == 0, n == 1

    // Seed the guess with a hardware sqrt to cut iteration count.
    // long double gives ~64-bit mantissa on x86, close enough that
    // Newton converges in 1-2 steps rather than dozens.
    __int128 x = (__int128)sqrtl((long double)n);
    if (x <= 0) {
        x = 1;
    }

    // Newton's method: x_{k+1} = (x_k + n/x_k) / 2
    // Converges monotonically down to floor(sqrt(n)) once it overshoots.
    __int128 y = (x + n / x) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    // x is now floor(sqrt(n)); guard against the seed undershooting.
    while ((x + 1) * (x + 1) <= n) {
        ++x;
    }
    return x;
}

static inline std::int64_t __attribute__((hot)) fixed_point_sqrt(std::int64_t a) {
    //Returns -1 if a is negative. The user is expected to set the value to be missing in that case
    if(a < 0) {
        return -1;
    }
    const __int128 n = (__int128)a * FIXED_POINT_FLOAT_SCALING_FACTOR;
    const __int128 s = __isqrt128(n); // floor(sqrt(n))

    // Round to nearest: compare n to s^2 and (s+1)^2 to see which is closer.
    const __int128 lo_err = n - s * s;
    const __int128 hi_err = (s + 1) * (s + 1) - n;
    const __int128 result = (hi_err < lo_err) ? (s + 1) : s;

    return (std::int64_t)result;
}
}