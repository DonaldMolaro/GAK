#pragma once

// Portable random helpers.
//
// The standard library fixes the output of std::mt19937 but not the
// algorithms behind std::uniform_int_distribution, std::uniform_real_distribution,
// std::bernoulli_distribution or std::shuffle, so the same seed produces
// different runs under libstdc++ (Linux) and libc++ (macOS). Everything in
// GAK draws through these helpers instead, which consume only raw mt19937
// output, so a fixed seed reproduces a run on every platform.
//
// Problem code that wants the same guarantee should use these helpers with
// `Population::randomEngine()` rather than the <random> distributions.

#include <cstdint>
#include <iterator>
#include <random>
#include <utility>

namespace gak
{

// Uniform integer in [low, high] (inclusive), by unbiased rejection sampling.
inline long uniformInt(std::mt19937& generator, long low, long high)
{
   const std::uint64_t range = static_cast<std::uint64_t>(high) - static_cast<std::uint64_t>(low) + 1u;
   if (range == 0u || high < low)
   {
      return low;
   }
   if (range <= 0x100000000ull)
   {
      const std::uint64_t limit = 0x100000000ull - (0x100000000ull % range);
      std::uint64_t draw;
      do
      {
         draw = static_cast<std::uint32_t>(generator());
      } while (draw >= limit);
      return low + static_cast<long>(draw % range);
   }
   const std::uint64_t limit = UINT64_MAX - (UINT64_MAX % range);
   std::uint64_t draw;
   do
   {
      const std::uint64_t upper = static_cast<std::uint32_t>(generator());
      const std::uint64_t lower = static_cast<std::uint32_t>(generator());
      draw = (upper << 32) | lower;
   } while (draw >= limit);
   return low + static_cast<long>(draw % range);
}

// Uniform integer in [0, upperBoundExclusive).
inline int uniformIndex(std::mt19937& generator, int upperBoundExclusive)
{
   return static_cast<int>(uniformInt(generator, 0, static_cast<long>(upperBoundExclusive) - 1));
}

// Uniform double in [0, 1) with 53 bits of precision.
inline double uniformUnit(std::mt19937& generator)
{
   const std::uint32_t upper = static_cast<std::uint32_t>(generator()) >> 5;
   const std::uint32_t lower = static_cast<std::uint32_t>(generator()) >> 6;
   return (upper * 67108864.0 + lower) * (1.0 / 9007199254740992.0);
}

// Uniform double in [low, high).
inline double uniformReal(std::mt19937& generator, double low, double high)
{
   return low + (high - low) * uniformUnit(generator);
}

// True with the given probability (always consumes one draw).
inline bool chance(std::mt19937& generator, double probability)
{
   return uniformUnit(generator) < probability;
}

// Fisher-Yates shuffle.
template <class RandomIt>
void shuffle(RandomIt first, RandomIt last, std::mt19937& generator)
{
   const long count = static_cast<long>(std::distance(first, last));
   for (long i = count - 1 ; i > 0 ; --i)
   {
      using std::swap;
      swap(first[i], first[uniformInt(generator, 0, i)]);
   }
}

}
