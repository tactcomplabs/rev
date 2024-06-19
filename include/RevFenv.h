//
// _RevFenv_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _REV_FENV_H_
#define _REV_FENV_H_

#include "RevInstTable.h"
#include <cfenv>
#include <cmath>
#include <stdexcept>

// GCC and Clang do not fully support FENV_ACCESS now. See:
//
// https://bugs.llvm.org/show_bug.cgi?id=8100
// https://gcc.gnu.org/legacy-ml/gcc-patches/2003-09/msg00104.html
// https://gcc.gcc.gnu.narkive.com/hDi1eOot/setting-frounding-math-by-default
// https://discourse.llvm.org/t/why-is-pragma-stdc-fenv-access-not-supported/46128/25
//
// Currently FP_MODE_FLAG in CMakeLists.txt is used as a workaround
//#pragma STDC FENV_ACCESS ON

namespace SST::RevCPU {

class RevFenv {
  FCSR&       fcsr;         // Reference to this register file's FCSR
  std::fenv_t saved_env{};  // The saved FP environment which is restored

public:
  /// Constructor saves Fenv state to be restored at destruction
  RevFenv( RevRegFile* R, FRMode rm, SST::Output* output ) : fcsr( R->GetFCSR() ) {

    // Save host's FP environment and set flags to default, non-trapping
    if( std::feholdexcept( &saved_env ) ) {
      throw std::runtime_error( "Getting floating-point environment with feholdexcept() is not working." );
    }

    // If the encoded rounding mode is dynamic, load the frm register
    if( rm == FRMode::DYN ) {
      rm = R->GetFRM();
    }

    // Set the Floating-Point Rounding Mode on the host
    int ret = 0;
    switch( rm ) {
    case FRMode::RNE:  // Round to Nearest, ties to Even
      ret = std::fesetround( FE_TONEAREST );
      break;

    case FRMode::RTZ:  // Round towards Zero
      ret = std::fesetround( FE_TOWARDZERO );
      break;

    case FRMode::RDN:  // Round Down (towards -Inf)
      ret = std::fesetround( FE_DOWNWARD );
      break;

    case FRMode::RUP:  // Round Up (towards +Inf)
      ret = std::fesetround( FE_UPWARD );
      break;

    case FRMode::RMM:  // Round to Nearest, ties to Max Magnitude
#ifdef FE_TONEARESTFROMZERO
      ret = std::fesetround( FE_TONEARESTFROMZERO );
#else
      output->fatal( CALL_INFO, -1, "Error: Round to nearest Max Magnitude not implemented at PC = 0x%" PRIx64 "\n", R->GetPC() );
#endif
      break;

    case FRMode::DYN:
      output->fatal( CALL_INFO, -1, "Illegal FCSR Rounding Mode of DYN at PC = 0x%" PRIx64 "\n", R->GetPC() );
      break;

    default: output->fatal( CALL_INFO, -1, "Unknown Rounding Mode at PC = 0x%" PRIx64 "\n", R->GetPC() ); break;
    }
    if( ret != 0 ) {
      output->fatal( CALL_INFO, -1, "Could not set FP rounding mode at PC = 0x%" PRIx64 "\n", R->GetPC() );
    }
  }

  /// Destructor sets FP exception flags and restores host FP Environment
  ~RevFenv() {
    // RISC-V does not support FP traps
    // Set the accumulated fflags based on exceptions
    int except = std::fetestexcept( FE_ALL_EXCEPT );
    if( except & FE_DIVBYZERO )
      fcsr = FCSR{ static_cast<uint32_t>( fcsr ) | static_cast<uint32_t>( FCSR::DZ ) };
    if( except & FE_INEXACT )
      fcsr = FCSR{ static_cast<uint32_t>( fcsr ) | static_cast<uint32_t>( FCSR::NX ) };
    if( except & FE_INVALID )
      fcsr = FCSR{ static_cast<uint32_t>( fcsr ) | static_cast<uint32_t>( FCSR::NV ) };
    if( except & FE_OVERFLOW )
      fcsr = FCSR{ static_cast<uint32_t>( fcsr ) | static_cast<uint32_t>( FCSR::OF ) };
    if( except & FE_UNDERFLOW )
      fcsr = FCSR{ static_cast<uint32_t>( fcsr ) | static_cast<uint32_t>( FCSR::UF ) };

    // Restore the host's saved FP Environment
    std::fesetenv( &saved_env );
  }

  // We allow moving, but not copying or assigning RevFenv.
  RevFenv( RevFenv&& )                 = default;
  RevFenv( const RevFenv& )            = delete;
  RevFenv& operator=( const RevFenv& ) = delete;
};  // RevFenv

}  // namespace SST::RevCPU

#endif
