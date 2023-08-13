#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#define WIDE_INTEGER_NAMESPACE ckormanyos

#include <mathematica_mathlink/mathematica_mathlink.h>
#include <math/wide_integer/uintwide_t.h>

namespace local
{
  using random_engine0_type = std::minstd_rand0;
  using random_engine1_type = std::minstd_rand;
  using random_engine2_type = std::mt19937_64;

  template<typename EntropyValueType,
           typename ClockType = std::chrono::high_resolution_clock>
  auto entropy64() -> EntropyValueType
  {
    using entropy_value_type = EntropyValueType;
    using clock_type         = ClockType;

    std::random_device dev;

    static std::mt19937_64 gen;

    const auto rnd32_lo = static_cast<std::uint32_t>(dev());
    const auto rnd32_hi = static_cast<std::uint32_t>(dev());

    const auto rnd64 =
      static_cast<std::uint64_t>
      (
          static_cast<std::uint64_t>(static_cast<std::uint64_t>(rnd32_hi) << static_cast<unsigned>(UINT8_C(32)))
        | static_cast<std::uint64_t>(rnd32_lo)
      );

    gen.seed(static_cast<typename std::mt19937_64::result_type>(rnd64));

    const auto rd = gen();

    const auto tp =
      static_cast<std::uint64_t>
      (
        std::chrono::duration_cast<std::chrono::nanoseconds>
        (
          clock_type::now().time_since_epoch()
        ).count()
      );

    return
      static_cast<entropy_value_type>
      (
        static_cast<std::uint64_t>(rd + tp)
      );
  }

  random_engine1_type eng0 { };
  random_engine1_type eng1 { };
  random_engine2_type eng2 { };

  auto dist0 = std::uniform_int_distribution<unsigned>
  {
    static_cast<unsigned>(UINT8_C(0)),
    static_cast<unsigned>(UINT8_C(1))
  };

  auto dist1 = std::uniform_int_distribution<unsigned>
  {
    static_cast<unsigned>(UINT8_C(1)),
    static_cast<unsigned>(UINT8_C(9))
  };

  auto dist2 = std::uniform_int_distribution<unsigned>
  {
    static_cast<unsigned>(UINT8_C(0)),
    static_cast<unsigned>(UINT8_C(9))
  };

  template<typename IntegralType>
  auto get_random_base10_integral_string(const IntegralType&) -> std::string
  {
    static auto seed_prescaler = static_cast<unsigned>(UINT8_C(0));

    auto str_result = std::string { };

    if(static_cast<unsigned>(seed_prescaler % static_cast<unsigned>(UINT16_C(2048))) == static_cast<unsigned>(UINT8_C(0)))
    {
      const auto ent0 = entropy64<std::uint64_t>();
      const auto ent1 = entropy64<std::uint64_t>();
      const auto ent2 = entropy64<std::uint64_t>();

      eng0.seed(static_cast<typename random_engine0_type::result_type>(ent0));
      eng1.seed(static_cast<typename random_engine1_type::result_type>(ent1));
      eng2.seed(static_cast<typename random_engine2_type::result_type>(ent2));
    }

    ++seed_prescaler;

    const auto is_neg = (dist0(eng0) == static_cast<unsigned>(UINT8_C(1)));

    if(is_neg) { str_result.push_back('-'); }

    const auto first_digit_as_char =
      static_cast<char>
      (
        static_cast<unsigned>
        (
          dist1(eng1) + static_cast<unsigned>(UINT8_C(0x30))
        )
      );

    str_result.push_back(first_digit_as_char);

    auto dist_len = std::uniform_int_distribution<unsigned>
    {
      static_cast<unsigned>(UINT8_C(2)),
      static_cast<unsigned>(static_cast<int>(std::numeric_limits<IntegralType>::digits10 - static_cast<int>(INT8_C(3))))
    };

    const auto len = dist_len(eng2);

    for(auto i = static_cast<unsigned>(UINT8_C(1)); i < len; ++i)
    {
      const auto next_digit_as_char =
        static_cast<char>
        (
          static_cast<unsigned>
          (
            dist2(eng2) + static_cast<unsigned>(UINT8_C(0x30))
          )
        );

      str_result.push_back(next_digit_as_char);
    }

    return str_result;
  }

  // Use the default mathlink 12.1 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\12.1\\MathKernel.exe\""
  };

  using mathematica_mathlink_type = mathematica::mathematica_mathlink<independent_test_system_mathlink_location>;

  using integral_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::int512_t;

  auto str_to_print_maker(const std::string&   str_left,
                          const std::string&   str_right,
                          const integral_type& first_ml,
                          const integral_type& first_wide,
                          const integral_type& second_ml,
                          const integral_type& second_wide,
                          const bool           result) -> std::string
  {
    std::stringstream strm;

    strm << "str_left                            : "                   << str_left     << '\n';
    strm << "str_right                           : "                   << str_right    << '\n';
    strm << "divmod_result_first_from_mathlink   : "                   << first_ml     << '\n';
    strm << "divmod_result_first_as_wide_integer : "                   << first_wide   << '\n';
    strm << "divmod_result_second_from_mathlink  : "                   << second_ml    << '\n';
    strm << "divmod_result_second_as_wide_integer: "                   << second_wide  << '\n';
    strm << "result_divmod_is_ok                 : " << std::boolalpha << result       << '\n';

    return strm.str();
  };
}

auto main() -> int
{
  local::mathematica_mathlink_type mlnk;

  auto result_total_is_ok = true;

  constexpr auto max_index = static_cast<std::uint32_t>(UINT32_C(4000000));
            auto run_index = static_cast<std::uint32_t>(UINT32_C(0));

  for( ; ((run_index < max_index) && result_total_is_ok); ++run_index)
  {
    const auto str_left  = local::get_random_base10_integral_string(local::integral_type { });
    const auto str_right = local::get_random_base10_integral_string(local::integral_type { });

    // Make commands like the following:

    // First[QuotientRemainder[87727206311238137505326963407487099090735132557053, -273791746447195436717]]
    // -320415817677533806983310533818

    // Last[QuotientRemainder[87727206311238137505326963407487099090735132557053, -273791746447195436717]]
    // -168305638431774838453

    const auto str_quot_rem   = "QuotientRemainder[" + str_left + "," + str_right + "]";
    const auto str_cmd_first  = "First[" + str_quot_rem + "]";
    const auto str_cmd_second = "Last [" + str_quot_rem + "]";

    auto str_rsp_first  = std::string { };
    auto str_rsp_second = std::string { };

    mlnk.send_command(str_cmd_first,  &str_rsp_first);
    mlnk.send_command(str_cmd_second, &str_rsp_second);

    const auto n_left  = local::integral_type { str_left.c_str() };
    const auto n_right = local::integral_type { str_right.c_str() };

    const auto divmod_result = divmod(n_left, n_right);

    const auto divmod_result_first_as_wide_integer = divmod_result.first;
    const auto divmod_result_first_from_mathlink   = local::integral_type(str_rsp_first.c_str());

    const auto divmod_result_second_as_wide_integer = divmod_result.second;
    const auto divmod_result_second_from_mathlink   = local::integral_type(str_rsp_second.c_str());

    const auto result_divmod_is_ok =
    (
         (divmod_result_first_as_wide_integer  == divmod_result_first_from_mathlink)
      && (divmod_result_second_as_wide_integer == divmod_result_second_from_mathlink)
    );

    result_total_is_ok = (result_divmod_is_ok && result_total_is_ok);

    {
      const auto str_to_print =
        local::str_to_print_maker
        (
          str_left,
          str_right,
          divmod_result_first_from_mathlink,
          divmod_result_first_as_wide_integer,
          divmod_result_second_from_mathlink,
          divmod_result_second_as_wide_integer,
          result_divmod_is_ok
        );

      if(static_cast<std::uint32_t>(run_index %  static_cast<std::uint32_t>(UINT16_C(2048))) == static_cast<std::uint32_t>(UINT32_C(0)))
      {
        std::cout << str_to_print << std::endl;
      }
      else
      {
        std::cout << str_to_print << '\n';
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
