///////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2018 -2019.                  //
//  Distributed under the Boost Software License,                //
//  Version 1.0. (See accompanying file LICENSE_1_0.txt          //
//  or copy at http://www.boost.org/LICENSE_1_0.txt)             //
///////////////////////////////////////////////////////////////////

#include <wide_integer/generic_template_uintwide_t.h>

bool wide_integer::example002_shl_shr()
{
  using uint256_t = wide_integer::generic_template::uint256_t;

  const uint256_t a("0xF4DF741DE58BCB2F37F18372026EF9CBCFC456CB80AF54D53BDEED78410065DE");

  const uint256_t c = (a << 67);
  const uint256_t d = (a >> 79);

  const bool result_is_ok = (   (c == "0xBF8C1B901377CE5E7E22B65C057AA6A9DEF76BC208032EF00000000000000000")
                             && (d == "0x1E9BEE83BCB17965E6FE306E404DDF3979F88AD97015E"));

  return result_is_ok;
}

// Enable this if you would like to activate this main() as a standalone example.
#if 0

#include <iomanip>
#include <iostream>

int main()
{
  const bool result_is_ok = wide_integer::example002_shl_shr();

  std::cout << "result_is_ok: " << std::boolalpha << result_is_ok << std::endl;
}

#endif
