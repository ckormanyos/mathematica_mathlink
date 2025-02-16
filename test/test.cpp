///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2022 - 2025.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define WIDE_INTEGER_NAMESPACE ckormanyos

#include <mathematica_mathlink/mathematica_mathlink.h>
#include <math/wide_integer/uintwide_t.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace prime_q
{
  // Use the default mathlink 14.0 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\14.0\\MathKernel.exe\""
  };

  ::std::uint32_t seed_prescaler { };
  ::std::uint64_t trials_total_times100 { };

  template<typename DistributionType,
           typename RandomEngineType,
           typename UnsignedIntegralType>
  auto set_prime_candidate(RandomEngineType& engine, DistributionType& dist, UnsignedIntegralType& prime_candidate) -> bool
  {
    using unsigned_integral_type = UnsignedIntegralType;
    using limb_type              = typename unsigned_integral_type::limb_type;

    bool result_set_n_is_ok { false };

    prime_candidate = dist(engine);

    const limb_type hi_limb { prime_candidate.crepresentation().back() };

    const ::std::uint8_t
      hi_nibble
      {
        static_cast<::std::uint8_t>
        (
          static_cast<::std::uint8_t>
          (
            hi_limb >> static_cast<unsigned>(::std::numeric_limits<limb_type>::digits - 4)
          ) & ::std::uint8_t { UINT8_C(0xF) }
        )
      };

    if(hi_nibble != ::std::uint8_t { UINT8_C(0) })
    {
      const limb_type lo_limb = prime_candidate.crepresentation().front();

      const ::std::uint8_t
        lo_bit
        {
          static_cast<::std::uint8_t>
          (
            static_cast<::std::uint8_t>(lo_limb) & ::std::uint8_t { UINT8_C(1) }
          )
        };

      // Remove all even candidates since they are non-prime.
      if(lo_bit != ::std::uint8_t { UINT8_C(0) })
      {
        const ::std::uint8_t
          lo_digit10
          {
            static_cast<::std::uint8_t>(prime_candidate % ::std::uint8_t { UINT8_C(10) })
          };

        // Continue by removing candidates having trailing digit 5.
        // The result is all candidates have trailing digit 1,3,7,9

        if(lo_digit10 != ::std::uint8_t { UINT8_C(5) })
        {
          // Now remove candidates having digital root 3, 6 or 9
          // because these are divisible by 3 and thus non-prime.

          // To compute the digital root of n dr(n), use
          // dr(n) = 1 + (n - 1) % 9.
          const unsigned_integral_type
            n_minus_one
            {
              prime_candidate - static_cast<limb_type>( unsigned { UINT8_C(1) })
            };

          const ::std::uint8_t
            digital_root
            {
              static_cast<::std::uint8_t>
              (
                  ::std::uint8_t { UINT8_C(1) }
                + static_cast<::std::uint8_t>(n_minus_one % ::std::uint8_t { UINT8_C(9) })
              )
            };

          const bool
            not_digital_root_of_3_6_or_9
            {
                 (digital_root != ::std::uint8_t { UINT8_C(3) })
              && (digital_root != ::std::uint8_t { UINT8_C(6) })
              && (digital_root != ::std::uint8_t { UINT8_C(9) })
            };

          if(not_digital_root_of_3_6_or_9)
          {
            // We have found a viable prime candidate. Use this prime
            // candidate to start a new Miller-Rabin primality test.

            result_set_n_is_ok = true;

            trials_total_times100 = std::uint64_t { trials_total_times100 + unsigned { UINT8_C(100) } };
          }
        }
      }
    }

    return result_set_n_is_ok;
  }

  template<typename IntegralTimePointType,
           typename ClockType = ::std::chrono::high_resolution_clock>
  auto time_point() -> IntegralTimePointType
  {
    using local_integral_time_point_type = IntegralTimePointType;
    using local_clock_type               = ClockType;

    const auto current_now =
      static_cast<::std::uintmax_t>
      (
        ::std::chrono::duration_cast<::std::chrono::nanoseconds>
        (
          local_clock_type::now().time_since_epoch()
        ).count()
      );

    return static_cast<local_integral_time_point_type>(current_now);
  }

  template<typename DistributionType,
           typename RandomEngineType1,
           typename RandomEngineType2,
           typename UnsignedIntegralType>
  auto get_pseudo_random_prime(DistributionType& dist, RandomEngineType1& generator1, RandomEngineType2& generator2, UnsignedIntegralType* p_prime = nullptr) -> void
  {
    using local_wide_integer_type = UnsignedIntegralType;

    local_wide_integer_type p0 { };

    for(;;)
    {
      while(!set_prime_candidate(generator1, dist, p0)) { ; }

      #if defined(WIDE_INTEGER_NAMESPACE)
      using local_unsigned_fast_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::unsigned_fast_type;
      #else
      using local_unsigned_fast_type = ::math::wide_integer::unsigned_fast_type;
      #endif

      const bool
        result_candidate_is_prime
        {
          miller_rabin(p0,
                       local_unsigned_fast_type { UINT8_C(25) },
                       dist,
                       generator2)
        };

      if(result_candidate_is_prime)
      {
        break;
      }

      ++seed_prescaler;

      const auto prescaler_mod = static_cast<::std::uint32_t>(seed_prescaler % static_cast<::std::uint32_t>(UINT16_C(1024)));

      if(prescaler_mod == static_cast<::std::uint32_t>(UINT8_C(0)))
      {
        using random_engine1_type = RandomEngineType1;
        using random_engine2_type = RandomEngineType2;

        using random_result1_type = typename random_engine1_type::result_type;
        using random_result2_type = typename random_engine2_type::result_type;

        generator1.seed(time_point<random_result1_type>());
        generator2.seed(time_point<random_result2_type>());
      }
    }

    if(p_prime != nullptr)
    {
      *p_prime = p0;
    }
  }
} // namespace prime_q

auto main() -> int;

auto main() -> int
{
  using random_engine1_type = ::std::linear_congruential_engine<::std::uint32_t, UINT32_C(48271), UINT32_C(0), UINT32_C(2147483647)>;
  using random_engine2_type = ::std::mt19937;

  random_engine1_type generator1 { prime_q::time_point<typename random_engine1_type::result_type>() };
  random_engine2_type generator2 { prime_q::time_point<typename random_engine2_type::result_type>() };

  #if defined(WIDE_INTEGER_NAMESPACE)
  using local_wide_integer_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uintwide_t<WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t { UINT16_C(256) }>;
  using local_distribution_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uniform_int_distribution<local_wide_integer_type::my_width2, typename local_wide_integer_type::limb_type>;
  #else
  using local_wide_integer_type = ::math::wide_integer::uintwide_t<math::wide_integer::size_t { UINT16_C(256) }>;
  using local_distribution_type = ::math::wide_integer::uniform_int_distribution<local_wide_integer_type::my_width2, typename local_wide_integer_type::limb_type>;
  #endif

  local_distribution_type
    dist
    {
      (::std::numeric_limits<local_wide_integer_type>::min)(),
      (::std::numeric_limits<local_wide_integer_type>::max)()
    };

  using local_mathematica_mathlink_type = mathematica::mathematica_mathlink<prime_q::independent_test_system_mathlink_location>;

  local_mathematica_mathlink_type mlnk { };

  bool result_total_is_ok { true };

  constexpr ::std::uint32_t max_index { ::std::uint32_t { UINT32_C(0x10000) } };

  ::std::uint32_t run_index { ::std::uint32_t { UINT32_C(0) } };

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    local_wide_integer_type prime_candidate { };

    prime_q::get_pseudo_random_prime(dist, generator1, generator2, &prime_candidate);

    const ::std::string str_prime_candidate { to_string(prime_candidate) };

    const ::std::string str_cmd { "PrimeQ[" + str_prime_candidate + "]" };

    ::std::string str_rsp { };

    mlnk.send_command(str_cmd, &str_rsp);

    const bool result_prime_candidate_is_ok { (str_rsp.find("True") != ::std::string::npos) };

    result_total_is_ok = (result_prime_candidate_is_ok && result_total_is_ok);

    const float
      ratio
      {
        static_cast<float>
        (
            prime_q::trials_total_times100
          / std::uint32_t { run_index + unsigned { UINT8_C(1) } }
        )
        / 100.0F
      };

    ::std::string str_report_this_prime { };

    {
      ::std::stringstream strm { };

      strm << "idx: "
           << std::setw(std::streamsize { INT8_C(9) })
           << std::right
           << run_index
           << ", p: "
           << std::setw(std::streamsize { std::numeric_limits<local_wide_integer_type>::digits10 + 1 })
           << std::right
           << str_prime_candidate
           ;

      str_report_this_prime = strm.str();
    }

    {
      ::std::stringstream strm { };

      strm << ", prime? "
           << ::std::boolalpha
           << result_prime_candidate_is_ok
           << ", pi': "
           << std::fixed
           << std::setprecision(2)
           << ratio
           ;

      str_report_this_prime += strm.str();
    }

    ::std::cout << str_report_this_prime << ::std::endl;
  }

  {
    result_total_is_ok = ((run_index == max_index) && result_total_is_ok);

    ::std::stringstream strm { };

    strm << "Summary                   : " << run_index        << " trials"          << '\n';
    strm << "result_total_is_ok        : " << ::std::boolalpha << result_total_is_ok << '\n';

    ::std::cout << ::std::endl << strm.str() << ::std::endl;
  }

  return (result_total_is_ok ? 0 : -1);
}
