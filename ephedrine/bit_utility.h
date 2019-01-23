#pragma once

/* a=target variable, b=bit number to act upon 0-n */
template <typename T1, typename T2>
constexpr void bit_set(T1 &a, T2 b) { ((a) |= (1ULL << (b))); }

template <typename T1, typename  T2>
constexpr void bit_clear(T1 &a, T2 b) { ((a) &= ~(1ULL << (b))); }

template <typename T1, typename T2>
constexpr void bit_flip(T1 &a, T2 b) { ((a) ^= (1ULL << (b))); }

template <typename T1, typename T2>
constexpr bool bit_check(const T1 a, T2 b)
{
	return (!!((a) & (1ULL << (b))));			// '!!' to make sure this returns 0 or 1
}

/* x=target variable, y=mask */
template<typename T1, typename T2>
constexpr void bitmask_set(T1 &x, T2 y) { ((x) |= (y)); }

template <typename T1, typename T2>
constexpr void bitmask_clear(T1 &x, T2 y) { ((x) &= (~(y))); }
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK_ALL(x,y) (((x) & (y)) == (y))   // warning: evaluates y twice
#define BITMASK_CHECK_ANY(x,y) ((x) & (y))