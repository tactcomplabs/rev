//
// _RevFenv_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _REV_FENV_H_
#define _REV_FENV_H_

#include <cmath>
#include <cfenv>
#include <stdexcept>

// GCC and Clang do not fully support FENV_ACCESS now. See:
//
// https://gcc.gnu.org/legacy-ml/gcc-patches/2003-09/msg00104.html
// https://gcc.gcc.gnu.narkive.com/hDi1eOot/setting-frounding-math-by-default
// https://bugs.llvm.org/show_bug.cgi?id=8100
// https://github.com/llvm/llvm-project/issues/8472
// https://discourse.llvm.org/t/why-is-pragma-stdc-fenv-access-not-supported/46128/25
//
// Currently FP_MODE_FLAG in CMakeLists.txt is used as a workaround
//#pragma STDC FENV_ACCESS ON

namespace SST::RevCPU{

// TODO: Right now we only need to save/restore rounding mode.
// Later, we may need to save and restore the entire fenv_t state
class RevFenv{
  FCSR& fcsr;
  std::fenv_t saved_env;

public:
  /// Constructor saves Fenv state to be restored at destruction
  explicit RevFenv(RevRegFile* R) : fcsr(R->GetFCSR()){
    // Save FP environment and set flags to default
    if(feholdexcept(&saved_env)){
      throw std::runtime_error("Getting floating-point environment "
                               "with feholdexcept() is not working.");
    }
  }

  /// Destructor sets flags and restores host FP Environment
  ~RevFenv(){
    // RISC-V does not support FP traps
    // Set the accumulated fflags based on exceptions
    int except = std::fetestexcept(FE_ALL_EXCEPT);
    if(except & FE_DIVBYZERO) fcsr.DZ = true;
    if(except & FE_INEXACT)   fcsr.NX = true;
    if(except & FE_INVALID)   fcsr.NV = true;
    if(except & FE_OVERFLOW)  fcsr.OF = true;
    if(except & FE_UNDERFLOW) fcsr.UF = true;

    // Restore the host's saved FP Environment
    fesetenv(&saved_env);
  }

  // We allow moving, but not copying RevFenv
  // This is to ensure that there is only a single copy
  // of the saved state at a time, similar to std::unique_ptr
  RevFenv(RevFenv&&) = default;
  RevFenv(const RevFenv&) = delete;

  // We disallow assigning
  RevFenv& operator=(const RevFenv&) = delete;
  RevFenv& operator=(RevFenv&&) = delete;

  // Get the FP rounding mode
  static int& GetFPRoundingMode(){
    thread_local int round = fegetround();
    return round;
  }

  // Set the FP rounding mode
  static void SetFPRoundingMode(int mode){
    int& round = GetFPRoundingMode();
    if(!fesetround(mode))
      round = mode;
  }
}; // RevFenv

} // namespace SST::RevCPU

#endif
