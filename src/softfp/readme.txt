SoftFP Library by Fabrice Bellard
=================================

1) Features
-----------

- Support of the IEEE 754-2008 32/64/128 bit floating point types.

- Support of the 4 elementary operations plus square root, fused
  multiply-add, minimum, maximum, conversion to and from 32/64/128
  bit signed or unsigned integers.

- RISCV FPU specifics are supported by default.

- 128 bit floating point type and 128 bit integers relying on the
  C compiler __int128 type.

2) SoftFP Test
--------------

The package provides the softfptest utility which tests the SoftFP
operations relative to the host x86_64 CPU or the softfloat
library. For convenvience, the softfloat library (extracted from QEMU)
is provided in the archive. Note that it is not part of the SoftFP
library.

3) License
----------

SoftFP is released under the MIT license.
