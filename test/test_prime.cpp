///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2022 - 2026.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define WIDE_INTEGER_NAMESPACE ckormanyos

//#if !defined(ML_USE_SOLOVAY_STRASSEN_PRIME_Q)
//#define ML_USE_SOLOVAY_STRASSEN_PRIME_Q
//#endif

#include <mathematica_mathlink/mathematica_mathlink.h>
#include <math/wide_integer/uintwide_t.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#if defined(ML_USE_SOLOVAY_STRASSEN_PRIME_Q)
namespace local_solovay_strassen {

namespace detail {

template<typename UnsignedIntegerType>
auto jacobi(UnsignedIntegerType a, UnsignedIntegerType n) -> int;

template<typename UnsignedIntegerType>
auto jacobi(UnsignedIntegerType a, UnsignedIntegerType n) -> int
{
  // Calculate the integer's Jacobi symbol.
  if(   ((static_cast<unsigned>(n) == 0U) && (n== 0U))
     || ((static_cast<unsigned>(n) % 2U) == 0U))
  {
    return 0;
  }

  a %= n;

  int result = 1;

  while(a != 0)
  {
    while((static_cast<unsigned>(a) % 2U) == 0U)
    {
      a /= 2U;

      UnsignedIntegerType r { n % 8U }; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

      if(   ((static_cast<unsigned>(r) == 3U) && (r == 3U))  // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
         || ((static_cast<unsigned>(r) == 5U) && (r == 5U))) // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
      {
        result = -result;
      }
    }

    std::swap(a, n);

    const unsigned a_mod_4 { static_cast<unsigned>(a % 4U) }; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    const unsigned n_mod_4 { static_cast<unsigned>(n % 4U) }; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    if((a_mod_4 == 3U) && (n_mod_4 == 3U)) // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    {
      result = -result;
    }

    a %= n;
  }

  const bool n_is_one { ((static_cast<unsigned>(n) == 1) && (n == 1U)) };

  return (n_is_one ? result : 0);
}

} // namespace detail

template<typename UnsignedIntegerType,
         typename DistributionType,
         typename GeneratorType>
auto solovay_strassen(const UnsignedIntegerType& n, const int iterations, DistributionType& distribution, GeneratorType& generator) -> bool; // NOLINT(readability-avoid-const-params-in-decls)

template<typename UnsignedIntegerType,
         typename DistributionType,
         typename GeneratorType>
auto solovay_strassen(const UnsignedIntegerType& n, const int iterations, DistributionType& distribution, GeneratorType& generator) -> bool // NOLINT(readability-avoid-const-params-in-decls)
{
  // Perform a Solovay-Strassen primality test.

  // If this ever goes to production, then testing a lot more semi-small
  // primes, as done in the library's Miller-Rabin, would make sense here.

  {
    const unsigned un { static_cast<unsigned>(n) };

    if((un <  2U) && (n <  2)) { return false; }
    if((un == 2U) && (n == 2)) { return true; }
    if((un %  2U) == 0U) { return false; }
  }

  using local_distribution_type = DistributionType;

  using local_wide_integer_type = UnsignedIntegerType;

  using local_param_type = typename DistributionType::param_type;

  const local_param_type
    params
    {
      local_wide_integer_type { unsigned { UINT8_C(2) } },
      local_wide_integer_type { n - unsigned { UINT8_C(1) } }
    };

  local_distribution_type dist { params };

  for(int i = 0; i < iterations; ++i)
  {
    local_wide_integer_type a { distribution(generator) };

    local_wide_integer_type g = gcd(a, n);

    if((static_cast<unsigned>(g) > 1) && (g > 1U))
    {
      return false;
    }

    const int jac = detail::jacobi(a, n);

    if(jac == 0)
    {
      return false;
    }

    local_wide_integer_type exponent { (n - 1) / 2 };
    local_wide_integer_type mod_exp { powm(a, exponent, n) };

    local_wide_integer_type jacobian { (jac == -1) ? (n - 1) : jac };

    if(mod_exp != (jacobian % n))
    {
      return false;
    }
  }

  // The candidate is probably prime.
  return true;
}

} // namespace local_solovay_strassen
#endif // ML_USE_SOLOVAY_STRASSEN_PRIME_Q

namespace prime_q
{
  // Use the default mathlink 14.0 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\14.0\\MathKernel.exe\""
  };

  using local_mathematica_mathlink_type = mathematica::mathematica_mathlink<independent_test_system_mathlink_location>;

  template<typename DistributionType,
           typename RandomEngineType,
           typename UnsignedIntegralType>
  auto set_prime_candidate(RandomEngineType& engine, DistributionType& dist, UnsignedIntegralType& prime_candidate) -> bool;

  template<typename IntegralTimePointType,
           typename ClockType = ::std::chrono::high_resolution_clock>
  auto time_point() -> IntegralTimePointType;

  template<typename DistributionType,
           typename RandomEngineType1,
           typename RandomEngineType2,
           typename UnsignedIntegralType>
  auto get_pseudo_random_prime(DistributionType& dist,
                               RandomEngineType1& generator1,
                               RandomEngineType2& generator2,
                               UnsignedIntegralType* p_prime = nullptr,
                               local_mathematica_mathlink_type* p_mlink = nullptr) -> bool;

  ::std::uint32_t seed_prescaler { };
  ::std::uint64_t trials_total_times1000 { };

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
      has_enough_hi_bits_check
      {
        static_cast<::std::uint8_t>
        (
          static_cast<::std::uint8_t>
          (
            hi_limb >> static_cast<unsigned>(::std::numeric_limits<limb_type>::digits - 4)
          ) & ::std::uint8_t { UINT8_C(0xE) }
        )
      };

    // Check for non-zero high bits when obtaining the next prime candidate.
    // This check ensure that the width of the prime candidate remains close
    // to the maximum of its type.

    if(has_enough_hi_bits_check != ::std::uint8_t { UINT8_C(0) })
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

            // Increment the trials-times-1000 counter by one new trial (add 1,000).
            trials_total_times1000 = std::uint64_t { trials_total_times1000 + unsigned { UINT16_C(1000) } };
          }
        }
      }
    }

    return result_set_n_is_ok;
  }

  template<typename IntegralTimePointType,
           typename ClockType>
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
  auto get_pseudo_random_prime(DistributionType& dist1,
                               RandomEngineType1& generator1,
                               RandomEngineType2& generator2,
                               UnsignedIntegralType* p_prime,
                               local_mathematica_mathlink_type* p_mlink) -> bool
  {
    using local_wide_integer_type = UnsignedIntegralType;

    local_wide_integer_type p0 { };

    bool result_total_is_ok { true };

    for(;;)
    {
      while(!set_prime_candidate(generator1, dist1, p0)) { ; }

      #if defined(WIDE_INTEGER_NAMESPACE)
      using local_unsigned_fast_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::unsigned_fast_type;
      #else
      using local_unsigned_fast_type = ::math::wide_integer::unsigned_fast_type;
      #endif

      #if defined(ML_USE_SOLOVAY_STRASSEN_PRIME_Q)
      constexpr local_unsigned_fast_type number_of_trials { UINT8_C(56) };
      #else
      constexpr local_unsigned_fast_type number_of_trials { UINT8_C(25) };
      #endif

      using local_distribution_type = DistributionType;

      local_distribution_type dist2 { local_wide_integer_type { 2U }, p0 - 1 };

      const bool
        result_candidate_is_prime
        {
          #if defined(ML_USE_SOLOVAY_STRASSEN_PRIME_Q)
          local_solovay_strassen::solovay_strassen
          #else
          miller_rabin
          #endif
          (
            p0,
            number_of_trials,
            dist2,
            generator2
          )
        };

      if(result_candidate_is_prime)
      {
        result_total_is_ok = (result_candidate_is_prime && result_total_is_ok);

        break;
      }
      else if(p_mlink != nullptr)
      {
        // If a non-null Mathlink object has been provided, then check
        // each suspected non-prime for non-primality also via Mathlink.

        const ::std::string str_non_prime_candidate { to_string(p0) };

        const ::std::string str_cmd { "PrimeQ[" + str_non_prime_candidate + "]" };

        ::std::string str_rsp { };

        p_mlink->send_command(str_cmd, &str_rsp);

        const bool result_non_prime_candidate_is_ok { (str_rsp.find("False") != ::std::string::npos) };

        result_total_is_ok = (result_non_prime_candidate_is_ok && result_total_is_ok);

        if(!result_total_is_ok)
        {
          ::std::cout << "\nError: p0: "
                      << p0
                      << "\nML disagrees with non-prime (the response was !False), and thus it is prime."
                      << "\nBut wide_integer did not properly identify the candidate to be prime.\n"
                      << std::endl;

          break;
        }
      }

      ++seed_prescaler;

      const auto seed_prescaler_mod1024 = static_cast<::std::uint32_t>(seed_prescaler % static_cast<::std::uint32_t>(UINT16_C(1024)));

      if(seed_prescaler_mod1024 == static_cast<::std::uint32_t>(UINT8_C(0)))
      {
        using random_engine1_type = RandomEngineType1;
        using random_engine2_type = RandomEngineType2;

        using random_engine1_result_type = typename random_engine1_type::result_type;
        using random_engine2_result_type = typename random_engine2_type::result_type;

        generator1.seed(time_point<random_engine1_result_type>());
        generator2.seed(time_point<random_engine2_result_type>());
      }
    }

    if(p_prime != nullptr)
    {
      *p_prime = p0;
    }

    return result_total_is_ok;
  }
} // namespace prime_q

auto main() -> int;

auto main() -> int
{
  using random_engine1_type = ::std::linear_congruential_engine<::std::uint32_t, UINT32_C(48271), UINT32_C(0), UINT32_C(2147483647)>;
  using random_engine2_type = ::std::mt19937_64;

  random_engine1_type generator1 { prime_q::time_point<typename random_engine1_type::result_type>() };
  random_engine2_type generator2 { prime_q::time_point<typename random_engine2_type::result_type>() };

  #if defined(WIDE_INTEGER_NAMESPACE)
  using local_wide_integer_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uintwide_t<WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t { UINT16_C(256) }>;
  using local_distribution_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uniform_int_distribution<local_wide_integer_type::my_width2, typename local_wide_integer_type::limb_type>;
  #else
  using local_wide_integer_type = ::math::wide_integer::uintwide_t<math::wide_integer::size_t { UINT16_C(256) }>;
  using local_distribution_type = ::math::wide_integer::uniform_int_distribution<local_wide_integer_type::my_width2, typename local_wide_integer_type::limb_type>;
  #endif

  // Select prime candidates from a range of 10^60 ... max(uint256_t) - 1.
  constexpr local_wide_integer_type
    dist_min
    {
      "1000000000000000000000000000000000000000000000000000000000000"
    };

  local_distribution_type
    dist
    {
      dist_min,
      (::std::numeric_limits<local_wide_integer_type>::max)() - 1
    };

  using prime_q::local_mathematica_mathlink_type;

  local_mathematica_mathlink_type mlnk { };

  bool result_total_is_ok { true };

  constexpr ::std::uint32_t max_index { ::std::uint32_t { UINT32_C(0x80000) } };

  ::std::uint32_t run_index { ::std::uint32_t { UINT32_C(0) } };

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    local_wide_integer_type prime_candidate { };

    const bool result_get_prime_candidate_is_ok { prime_q::get_pseudo_random_prime(dist, generator1, generator2, &prime_candidate, &mlnk) };

    result_total_is_ok = (result_get_prime_candidate_is_ok && result_total_is_ok);

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
            prime_q::trials_total_times1000
          / std::uint32_t { run_index + unsigned { UINT8_C(1) } }
        )
        / 1000.0F
      };

    ::std::string str_report_this_prime { };

    {
      ::std::stringstream strm { };

      strm << "trial: "
           << std::setw(std::streamsize { INT8_C(9) })
           << std::right
           << (run_index + 1)
           << ", p: "
           << std::setw(std::streamsize { std::numeric_limits<local_wide_integer_type>::digits10 + 1 })
           << std::right
           << str_prime_candidate
           << ", prime? "
           << ::std::boolalpha
           << result_prime_candidate_is_ok
           << ", pi': "
           << std::fixed
           << std::setprecision(2)
           << ratio
           ;

      str_report_this_prime = strm.str();
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
