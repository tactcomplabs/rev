//
// _RevFeature_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVFEATURE_H_
#define _SST_REVCPU_REVFEATURE_H_

#include <cstdint>
#include <string>

// -- SST Headers
#include "SST.h"

namespace SST::RevCPU {

/// Table of RISC-V extension flags. They must be powers of two which can be
/// ORed to indicate multiple extensions being present.
enum RevFeatureType : uint32_t {
  RV_UNKNOWN  = 0,        ///< RevFeatureType: unknown feature
  RV_I        = 1 << 0,   ///< RevFeatureType: I-extension
  RV_E        = 1 << 1,   ///< RevFeatureType: E-extension
  RV_M        = 1 << 2,   ///< RevFeatureType: M-extension
  RV_F        = 1 << 3,   ///< RevFeatureType: F-extension
  RV_D        = 1 << 4,   ///< RevFeatureType: D-extension
  RV_Q        = 1 << 5,   ///< RevFeatureType: Q-extension
  RV_C        = 1 << 6,   ///< RevFeatureType: C-extension
  RV_B        = 1 << 7,   ///< RevFeatureType: C-extension
  RV_P        = 1 << 8,   ///< RevFeatureType: P-Extension
  RV_V        = 1 << 9,   ///< RevFeatureType: V-extension
  RV_H        = 1 << 10,  ///< RevFeatureType: H-extension
  RV_ZICBOM   = 1 << 11,  ///< RevFeatureType: Zicbom-extension
  RV_ZICNTR   = 1 << 12,  ///< RevFeatureType: Zicntr-extension
  RV_ZICSR    = 1 << 13,  ///< RevFEatureType: Zicsr-extension
  RV_ZIFENCEI = 1 << 14,  ///< RevFeatureType: Zifencei-extension
  RV_ZMMUL    = 1 << 15,  ///< RevFeatureType: Zmmul-extension
  RV_ZAAMO    = 1 << 16,  ///< RevFeatureType: Zaamo-extension
  RV_ZALRSC   = 1 << 17,  ///< RevFeatureType: Zalrsc-extension
  RV_ZFA      = 1 << 18,  ///< RevFeatureType: Zfa-extension
  RV_ZFH      = 1 << 19,  ///< RevFeatureType: H-extension
  RV_ZFHMIN   = 1 << 20,  ///< RevFeatureRtpe: Zfhmin extension
  RV_ZTSO     = 1 << 21,  ///< RevFeatureType: Ztso-extension
};

struct RevFeature {
  /// RevFeature: standard constructor
  RevFeature( std::string Machine, SST::Output* Output, unsigned Min, unsigned Max, unsigned Id, bool randomizeCosts );

  /// RevFeature: standard destructor
  ~RevFeature()                              = default;

  /// RevFeature: deleted copy constructor
  RevFeature( const RevFeature& )            = delete;

  /// RevFeature: deleted copy assignment operator
  RevFeature& operator=( const RevFeature& ) = delete;

  /// IsModeEnabled: determines if the target mode is enabled
  bool IsModeEnabled( RevFeatureType Type ) const { return ( features & Type ) == Type; }

  /// SetMachineEntry: set the machine model item
  void SetMachineEntry( RevFeatureType Type ) { features = RevFeatureType{ features | Type }; }

  /// GetMachineModel: retrieve the feature string
  auto GetMachineModel() const { return machine; }

  /// GetFeatures: retrieve the feature encoding
  auto GetFeatures() const { return features; }

  /// GetMinCost: get the minimum cost
  auto GetMinCost() const { return MinCost; }

  /// GetMaxCost: get the maximum cost
  auto GetMaxCost() const { return MaxCost; }

  /// IsRV64: Is the device an RV64
  bool IsRV64() const { return xlen >= 64; }

  /// HasF: Does the device support F?
  bool HasF() const { return IsModeEnabled( RV_F ); }

  /// HasD: Does the device support D?
  bool HasD() const { return IsModeEnabled( RV_D ); }

  /// HasCompressed: Returns whether RV32 or RV64 "C" is enabled
  bool HasCompressed() const { return IsModeEnabled( RV_C ); }

  /// GetProcID: Retrieve the ProcID of the target object
  auto GetProcID() const { return ProcID; }

  /// GetHartToExecID: Retrieve the current executing Hart
  uint16_t GetHartToExecID() const { return HartToExecID; }

  /// SetHartToExecID: Set the current executing Hart
  void SetHartToExecID( unsigned hart ) { HartToExecID = hart; }

  /// GetRandomizeCosts: Return whether to randomize costs
  bool GetRandomizeCosts() const { return randomizeCosts; }

private:
  const std::string  machine;                 ///< RevFeature: feature string
  SST::Output* const output;                  ///< RevFeature: output handler
  const unsigned     MinCost;                 ///< RevFeature: min memory cost
  const unsigned     MaxCost;                 ///< RevFeature: max memory cost
  const unsigned     ProcID;                  ///< RevFeature: RISC-V Proc ID
  unsigned           HartToExecID{};          ///< RevFeature: The current executing Hart on RevCore
  RevFeatureType     features{ RV_UNKNOWN };  ///< RevFeature: feature elements
  unsigned           xlen{};                  ///< RevFeature: RISC-V Xlen
  const bool         randomizeCosts;          ///< RevFeature: Whether to randomize costs
  bool               ParseMachineModel();     ///< RevFeature: Parse the machine model string

};  // class RevFeature

}  // namespace SST::RevCPU

#endif

// EOF
