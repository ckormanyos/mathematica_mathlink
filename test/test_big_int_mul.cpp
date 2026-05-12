///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2026.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <mathematica_mathlink/mathematica_mathlink.h>
#include <beman/big_int/big_int.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace local {

namespace detail {

namespace bmp {

namespace detail {

// Generates a random hex string for a size of exactly `bits` bits,
// meaning the MSB is 1 (so the value is in [2^(bits-1), 2^bits)).
// `bits == 0` returns "0".
[[nodiscard]] inline std::string random_hex_of_bits(std::mt19937_64& rng, const std::size_t bits)
{
  static constexpr char table[] = "0123456789abcdef";

  if (bits == 0)
  {
    return std::string{"0"};
  }

  const std::size_t num_hex  = (bits + 3) / 4;
  const std::size_t top_bits = bits - (num_hex - 1) * 4; // 1..4

  std::string s;
  s.reserve(num_hex);

  const unsigned                          top_high = 1u << (top_bits - 1);
  std::uniform_int_distribution<unsigned> top_rand(0, top_high - 1);
  s.push_back(table[top_high | top_rand(rng)]);

  std::uniform_int_distribution<unsigned> any(0, 15);

  for (std::size_t i = 1; i < num_hex; ++i)
  {
    s.push_back(table[any(rng)]);
  }

  return s;
}

} // namespace detail

[[nodiscard]]
inline std::string random_big_int(const std::size_t bits, const bool negative = false)
{
  static std::mt19937_64 rng{std::random_device{}()};

  if(bits == 0)
  {
    return std::string{"0"};
  }

  std::string signed_hex;

  signed_hex.reserve(((bits + 3) / 4) + 1);

  if(negative)
  {
    signed_hex.push_back('-');
  }

  signed_hex += detail::random_hex_of_bits(rng, bits);

  return signed_hex;
}

} // namespace bmp

using str_pair_type = std::pair<std::string, std::string>;

using random_engine_length_type = std::minstd_rand;

random_engine_length_type
  generator_bits_length
  {
    static_cast<typename random_engine_length_type::result_type>(std::random_device{}())
  };

inline constexpr std::size_t
  limb_bits
  {
    static_cast<std::size_t>(std::numeric_limits<::beman::big_int::uint_multiprecision_t>::digits)
  };

std::uniform_int_distribution
  distribution_bits_length
  {
    std::size_t{700U} * static_cast<std::size_t>(limb_bits),
    std::size_t{900U} * static_cast<std::size_t>(limb_bits)
  };

auto get_hex_string_pair() -> std::pair<std::string, std::string>;

auto get_hex_string_pair() -> std::pair<std::string, std::string>
{
  std::size_t len_a_in_bits{};
  std::size_t len_b_in_bits{};

  static unsigned seed_prescalar{};
  static unsigned seed_increment{};

  // On a pre-defined schedule, seed the random length generator with fixed values.
  if((++seed_prescalar % 128U) == 0U)
  {
    detail::generator_bits_length.seed(std::random_device{}());
  }

  len_a_in_bits = detail::distribution_bits_length(detail::generator_bits_length);
  len_b_in_bits = detail::distribution_bits_length(detail::generator_bits_length);

  const std::string str_a{bmp::random_big_int(len_a_in_bits)};
  const std::string str_b{bmp::random_big_int(len_b_in_bits)};

  return {str_a, str_b};
}

} // namespace detail

// Use the default mathlink 12.1 kernel location on Win*.
constexpr char independent_test_system_mathlink_location[]
{
  "\"C:\\Program Files\\Wolfram Research\\Mathematica\\12.1\\MathKernel.exe\""
};

using mathematica_mathlink_type = mathematica::mathematica_mathlink<independent_test_system_mathlink_location>;

using integral_type = beman::big_int::big_int;

} // namespace local

auto main() -> int
{
  local::mathematica_mathlink_type mlnk;

  auto result_total_is_ok = true;

  constexpr auto max_index = static_cast<std::uint32_t>(UINT32_C(131072));
            auto run_index = static_cast<std::uint32_t>(UINT32_C(0));

  std::uint64_t elapsed_total_muls { };

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    const local::detail::str_pair_type str_pair { local::detail::get_hex_string_pair() };

    // Make commands like the following:

    local::integral_type bn_a { };
    local::integral_type bn_b { };

    const auto fc_result_a { from_chars(str_pair.first.data(), str_pair.first.data() + str_pair.first.size(), bn_a, 16) };
    const auto fc_result_b { from_chars(str_pair.second.data(), str_pair.second.data() + str_pair.second.size(), bn_b, 16) };

    static_cast<void>(fc_result_a);
    static_cast<void>(fc_result_b);

    // 87727206311238137505326963407487099090735132557053*273791746447195436717

    const std::string str_cmd_mul { to_string(bn_a) + "*" + to_string(bn_b) };

    std::string str_rsp_mul { };

    mlnk.send_command(str_cmd_mul, &str_rsp_mul);

    const auto start { std::chrono::high_resolution_clock::now() };

    const local::integral_type mul_result = bn_a * bn_b;

    const auto stop { std::chrono::high_resolution_clock::now() };

    const auto elapsed_one_mul { std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() };

    elapsed_total_muls += elapsed_one_mul;

    const bool result_mul_is_ok { (str_rsp_mul  == to_string(mul_result)) };

    result_total_is_ok = (result_mul_is_ok && result_total_is_ok);

    {
      if((run_index > 0U) && ((run_index % 32U) == UINT32_C(0)))
      {
        const double average_mul_time_us = (static_cast<double>(elapsed_total_muls) / static_cast<double>(run_index)) / 1000.0;

        std::cout << "run_index: " << run_index << ", average_mul_time_us: " << std::setprecision(1) << std::fixed << average_mul_time_us << std::endl;
      }
    }
  }

  result_total_is_ok = ((run_index == max_index) && result_total_is_ok);

  {
    std::stringstream strm;

    strm << '\n';
    strm << "Summary                            : " << run_index      << " trials"           << '\n';
    strm << "result_total_is_ok                 : " << std::boolalpha << result_total_is_ok  << '\n';

    std::cout << strm.str() << std::endl;
  }

  return (result_total_is_ok ? 0 : -1);
}
