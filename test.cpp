#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

#define WIDE_INTEGER_NAMESPACE ckormanyos

#include <mathematica_mathlink.h>
#include <math/wide_integer/uintwide_t.h>

namespace local
{
  // Use the default mathlink 12.1 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\12.1\\MathKernel.exe\""
  };

  #if defined(WIDE_INTEGER_NAMESPACE)
  using wide_integer_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uintwide_t<static_cast<WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t>(UINT32_C(512))>;
  using distribution_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #else
  using wide_integer_type = ::math::wide_integer::uintwide_t<static_cast<math::wide_integer::size_t>(UINT32_C(512))>;
  using distribution_type = ::math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #endif

  using random_engine1_type = std::linear_congruential_engine<std::uint32_t, UINT32_C(48271), UINT32_C(0), UINT32_C(2147483647)>;
  using random_engine2_type = std::mt19937;

  template<typename IntegralTimePointType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto time_point() -> IntegralTimePointType
  {
    using local_integral_time_point_type = IntegralTimePointType;
    using local_clock_type               = ClockType;

    const auto current_now =
      static_cast<std::uintmax_t>
      (
        std::chrono::duration_cast<std::chrono::nanoseconds>
        (
          local_clock_type::now().time_since_epoch()
        ).count()
      );

    return static_cast<local_integral_time_point_type>(current_now);
  }

  std::uint32_t seed_prescaler;

  random_engine1_type generator1(time_point<typename random_engine1_type::result_type>());
  random_engine2_type generator2(time_point<typename random_engine2_type::result_type>());

  auto distribution1 = distribution_type { };
  auto distribution2 = distribution_type { };

  auto get_pseudo_random_prime(wide_integer_type* p_prime = nullptr) -> void
  {
    auto p0 = wide_integer_type { };

    for(;;)
    {
      p0 = distribution1(generator1);

      const auto is_prime_candidate = miller_rabin(p0,
                                                   25U,
                                                   distribution2,
                                                   generator2);

      if(is_prime_candidate)
      {
        break;
      }

      const auto prescaler_mod = static_cast<std::uint32_t>(++seed_prescaler % static_cast<std::uint32_t>(UINT16_C(1024)));

      const auto do_seed_generators = (prescaler_mod == static_cast<std::uint32_t>(UINT8_C(0)));

      if(do_seed_generators)
      {
        generator1.seed(time_point<typename random_engine1_type::result_type>());
        generator2.seed(time_point<typename random_engine2_type::result_type>());
      }
    }

    if(p_prime != nullptr)
    {
      *p_prime = p0;
    }
  }
}

auto main() -> int
{
  using local_mathematica_mathlink_type = mathematica::mathematica_mathlink<local::independent_test_system_mathlink_location>;

  local_mathematica_mathlink_type mlnk;

  using local_integral_type = ckormanyos::math::wide_integer::uint256_t;

  auto result_total_is_ok = true;

  auto max_index = static_cast<std::uint32_t>(UINT32_C(8192));
  auto run_index = static_cast<std::uint32_t>(UINT32_C(0));

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    auto prime_candidate = local::wide_integer_type { };

    local::get_pseudo_random_prime(&prime_candidate);

    const auto str_prime_candidate = to_string(prime_candidate);

    const auto str_cmd = "PrimeQ[" + str_prime_candidate + "]";

    std::string str_rsp;

    mlnk.send_command(str_cmd, &str_rsp);

    const auto result_prime_candidate_is_ok = (str_rsp.find("True") != std::string::npos);

    result_total_is_ok = (result_prime_candidate_is_ok && result_total_is_ok);

    std::cout << "run_index: "
              << run_index
              << ", prime candidate : "
              << str_prime_candidate
              << ", is prime? "
              << std::boolalpha
              << result_prime_candidate_is_ok
              << std::endl;
  }

  result_total_is_ok = ((run_index == max_index) && result_total_is_ok);

  {
    std::cout << std::endl;
    std::cout << "Summary                   : " << run_index      << " trials"          << std::endl;
    std::cout << "result_total_is_ok        : " << std::boolalpha << result_total_is_ok << std::endl;
  }

  return (result_total_is_ok ? 0 : -1);
}
