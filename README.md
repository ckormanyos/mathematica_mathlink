mathematica_mathlink
==================

mathematica_mathlink provides a C++ template header for interacting with the Mathematica(R) kernel.

## Implementation Goals

  - Interact directly with the Mathematica(R) kernel by sending/receiving _packets_.
  - Clean header-only C++14 design.
  - Seamless portability to any modern C++14, 17, 20, 23 compiler and beyond.

## Examples

Several completely worked out examples are available in the repository.

  - [test_bessel_j_versus_boost.cpp](./test/test_bessel_j_versus_boost.cpp) tests a variety of cylindrical Bessel function values comparing them with results from the kernel.
  - [test_divmod.cpp](./test/test_divmod.cpp) generates random wide integers and tests the `divmod` function versus the kernel. This function is equivalent to Python-3's double-divide (`//`) function or Mathematica(R)'s `QuotientRemainder` function. This test program requires the [ckormanyos/wide-integer](https://github.com/ckormanyos/wide-integer) header-only C++ _wide_-_integer_ library.
  - [test_prime.cpp](./test/test_prime.cpp) generate random wide-integer prime numbers and test them with the kernel. This test program requires the [ckormanyos/wide-integer](https://github.com/ckormanyos/wide-integer) header-only C++ _wide_-_integer_ library.

## Additional information

  - At the moment, mathematica_mathlink can only handle single return packets.
  - The path strings for finding/locating the Mathematica(R) kernel are only available for `Win*`, not for `*nix`.
  - Link with wstp64i4.lib and run in the presence of wstp64i4.dll (proprietary libraries).
