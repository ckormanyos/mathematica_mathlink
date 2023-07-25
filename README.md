mathematica_mathlink
==================

mathematica_mathlink provides a C++ template header for interacting with the Mathematica(R) kernel.

## Implementation Goals

  - Interact directly with the Mathematica(R) kernel by sending/receiving _packets_.
  - Clean header-only C++14 design.
  - Seamless portability to any modern C++14, 17, 20, 23 compiler and beyond.

## Examples

Several completely worked out examples are available in the repository.
These include the following.

TBD.

## Known Limitations

  - At the moment, mathematica_mathlink can only handle single return packets.
  - The path strings for finding/locating the Mathematica(R) kernel are only available for `Win*`, not for `*nix`.
