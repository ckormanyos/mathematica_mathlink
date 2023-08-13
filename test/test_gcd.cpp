///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2022 - 2023.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iomanip>
#include <iostream>
#include <random>

#define WIDE_INTEGER_NAMESPACE ckormanyos

#include <mathematica_mathlink/mathematica_mathlink.h>
#include <math/wide_integer/uintwide_t.h>

namespace local
{
  // Use the default mathlink 12.1 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\12.1\\MathKernel.exe\""
  };

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

  auto seed_prescaler = std::uint32_t { };

  using random_engine1_type = std::linear_congruential_engine<std::uint32_t, UINT32_C(48271), UINT32_C(0), UINT32_C(2147483647)>;
  using random_engine2_type = std::mt19937;

  auto generator1 = random_engine1_type { time_point<typename random_engine1_type::result_type>() };
  auto generator2 = random_engine2_type { time_point<typename random_engine2_type::result_type>() };

  #if defined(WIDE_INTEGER_NAMESPACE)
  using wide_integer_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uintwide_t<static_cast<WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t>(UINT32_C(256))>;
  using distribution_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #else
  using wide_integer_type = ::math::wide_integer::uintwide_t<static_cast<math::wide_integer::size_t>(UINT32_C(256))>;
  using distribution_type = ::math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #endif

  auto distribution1 = distribution_type { };
  auto distribution2 = distribution_type { };

  auto get_pseudo_random_wide_integers(wide_integer_type* u, wide_integer_type* v) -> void
  {
    *u = distribution1(generator1);
    *v = distribution2(generator2);

    ++seed_prescaler;

    const auto prescaler_mod = static_cast<std::uint32_t>(seed_prescaler % static_cast<std::uint32_t>(UINT16_C(1024)));

    if(prescaler_mod == static_cast<std::uint32_t>(UINT8_C(0)))
    {
      using random_result1_type = typename random_engine1_type::result_type;
      using random_result2_type = typename random_engine2_type::result_type;

      generator1.seed(time_point<random_result1_type>());
      generator2.seed(time_point<random_result2_type>());
    }
  }

  class gcd_holder
  {
  public:
    explicit gcd_holder(const wide_integer_type& u = wide_integer_type(),
                        const wide_integer_type& v = wide_integer_type())
      : left (u),
        right(v) { }

    auto get_result() const noexcept -> const wide_integer_type&
    {
      return gcd_result;
    }

    auto get_u() const -> const wide_integer_type& { return left; }
    auto get_v() const -> const wide_integer_type& { return right; }

    auto set_u(const wide_integer_type& u) -> void { left  = u; }
    auto set_v(const wide_integer_type& v) -> void { right = v; }

    auto compute() -> void { gcd_result = gcd(left, right); }

  private:
    wide_integer_type left       { };
    wide_integer_type right      { };
    wide_integer_type gcd_result { };
  };
} // namespace local

auto main() -> int;

auto main() -> int
{
  using local_mathematica_mathlink_type = mathematica::mathematica_mathlink<local::independent_test_system_mathlink_location>;

  local_mathematica_mathlink_type mlnk;

  auto result_total_is_ok = true;

  auto max_index = static_cast<std::uint32_t>(UINT32_C(0x400000));
  auto run_index = static_cast<std::uint32_t>(UINT32_C(0));

  const auto flg = std::cout.flags();

  local::gcd_holder gcd_holder_max(static_cast<unsigned>(UINT8_C(0)));

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    auto u = local::wide_integer_type { };
    auto v = local::wide_integer_type { };

    local::get_pseudo_random_wide_integers(&u, &v);

    local::gcd_holder gh(u, v);

    gh.compute();

    const auto str_cmd = "GCD[" + to_string(gh.get_u()) + "," + to_string(gh.get_v()) + "]";

    auto str_rsp = std::string { };

    mlnk.send_command(str_cmd, &str_rsp);

    const auto result_gcd_is_ok = (local::wide_integer_type(str_rsp.c_str()) == gh.get_result());

    if(gh.get_result() > gcd_holder_max.get_result())
    {
      gcd_holder_max = gh;

      std::cout << "max GCD:"
                << "\nu: " << gcd_holder_max.get_u() << '\n'
                << "v: " << gcd_holder_max.get_v()
                << "\ngcd(u, v) : "
                << gcd_holder_max.get_result() << '\n';
    }

    result_total_is_ok = (result_gcd_is_ok && result_total_is_ok);
  }

  result_total_is_ok = ((run_index == max_index) && result_total_is_ok);

  std::cout << "\nSummary                 : " << run_index      << " trials\n";
  std::cout << "result_total_is_ok        : " << std::boolalpha << result_total_is_ok << std::endl;

  std::cout.flags(flg);

  return (result_total_is_ok ? 0 : -1);
}
