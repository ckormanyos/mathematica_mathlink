#include <iomanip>
#include <iostream>

#include <mathematica_mathlink/mathematica_mathlink.h>

#include <boost/math/special_functions/bessel.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace local
{
  // Use the default mathlink 12.1 kernel location on Win*.
  constexpr char independent_test_system_mathlink_location[]
  {
    "\"C:\\Program Files\\Wolfram Research\\Mathematica\\12.1\\MathKernel.exe\""
  };
}

auto main() -> int
{
  using local_mathematica_mathlink_type = mathematica::mathematica_mathlink<local::independent_test_system_mathlink_location>;

  local_mathematica_mathlink_type mlnk;

  // N[BesselJ[((i + 128) / 97), (34/10)], 101]
  // for i = 0...31

  for(auto   ui_add = static_cast<unsigned>(UINT8_C(0));
             ui_add < static_cast<unsigned>(UINT8_C(32));
           ++ui_add)
  {
    const auto ui_val = static_cast<unsigned>(static_cast<unsigned>(UINT8_C(128)) + ui_add);

    const auto str_cmd = "N[BesselJ[(" + std::to_string(ui_val) + " / 97), (34/10)], 101]";

    std::string str_rsp;

    mlnk.send_command(str_cmd, &str_rsp);

    std::cout << str_rsp << std::endl;

    using multiprecision_type = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<101>, boost::multiprecision::et_off>;

    const auto jv =
      boost::math::cyl_bessel_j
      (
        static_cast<multiprecision_type>(static_cast<multiprecision_type>(ui_val) / 97U),
        static_cast<multiprecision_type>(static_cast<multiprecision_type>(34U) / 10U)
      );

    std::cout << std::setprecision(static_cast<std::streamsize>(std::numeric_limits<multiprecision_type>::digits10))
              << jv
              << std::endl;
  }
}
