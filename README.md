mathematica_mathlink
==================

<p align="center">
    <a href="https://github.com/ckormanyos/mathematica_mathlink/actions">
        <img src="https://github.com/ckormanyos/mathematica_mathlink/actions/workflows/mathematica_mathlink.yml/badge.svg" alt="Build Status"></a>
</p>

`ckormanyos/mathematica_mathlink` provides a C++ template header
for interacting with the Mathematica(R) kernel.

## Implementation Goals

  - Interact directly with the Mathematica(R) kernel by sending/receiving _packets_.
  - Clean header-only C++14 design.
  - Seamless portability to any modern C++14, 17, 20, 23 compiler and beyond.
  - Support an easy-to-use MSVC solution for simple build.
  - Include a selection of non-trivial examples.

## Test Examples

Several completely worked out test examples are available in the
[`test`](./test) directory of the repository.

  - [test_bessel_j_versus_boost.cpp](./test/test_bessel_j_versus_boost.cpp) tests a variety of high-precision cylindrical Bessel function values comparing them with results from the kernel. This test requires the `Boost.Math` and `Boost.Multiprecision` libraries.
  - [test_divmod.cpp](./test/test_divmod.cpp) generates pseudo-random wide integers and tests the `divmod` function versus the kernel. This function is equivalent to Python-3's double-divide (`//`) function or Mathematica(R)'s `QuotientRemainder` function. This test program requires the [ckormanyos/wide-integer](https://github.com/ckormanyos/wide-integer) header-only C++ _wide_-_integer_ library.
  - [test_gcd.cpp](./test/test_gcd.cpp) generate pairs of pseudo-random wide-integers, compute their `gcd` (GCD, greatest common divisor) and confirm the GCD results with the kernel. This test program requires the [ckormanyos/wide-integer](https://github.com/ckormanyos/wide-integer) header-only C++ _wide_-_integer_ library.
  - [test_prime.cpp](./test/test_prime.cpp) generate pseudo-random wide-integer prime numbers and verify their primality with the kernel. This test program requires the [ckormanyos/wide-integer](https://github.com/ckormanyos/wide-integer) header-only C++ _wide_-_integer_ library.

## Additional information

  - At the moment, `ckormanyos/mathematica_mathlink` can only handle single return packets.
  - The path strings for finding/locating the Mathematica(R) kernel are only available for `Win*`, not for `*nix`.
  - Link with `wstp64i4.lib` and run in the presence of `wstp64i4.dll` (which are proprietary libraries).
  - CI compiles the test examples to object only (and does not link). Compilation is checked using GCC and clang compilers.
