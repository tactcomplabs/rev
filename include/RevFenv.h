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

#include <cfenv>
#include <memory>

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
  int saved_round;      ///< Saved Floating-Point Rounding Mode

  // We use Meyers singleton to avoid initialization order fiasco
  static int& round(){
    thread_local int round = fegetround();
    return round;
  }

public:

  /// Constructor saves Fenv state
  RevFenv() : saved_round(round()) {
    if(saved_round < 0){
      throw std::runtime_error("Getting floating-point rounding mode with fegetround() is not working.");
    }
  }

  /// Destructor restores Fenv state
  ~RevFenv() { SetRound(saved_round); }

  // We allow moving, but not copying RevFenv
  // This is to ensure that there is only a single copy
  // of the saved state at a time, similar to std::unique_ptr
  RevFenv(RevFenv&&) = default;
  RevFenv(const RevFenv&) = delete;

  // We disallow assigning
  RevFenv& operator=(const RevFenv&) = delete;
  RevFenv& operator=(RevFenv&&) = delete;

  // Get the current FP rounding state
  static int GetRound(){
    return round();
  }

  // Set the FP rounding state if it differs from current
  static int SetRound(int mode){
    int rc = 0;
    if(mode != round()){
      rc = fesetround(mode);
      if(rc == 0){
        round() = mode;
      }
    }
    return rc;
  }
}; // RevFenv

} // namespace SST::RevCPU

#endif
