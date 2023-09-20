//
// _RevFeature_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVFEATURE_H_
#define _SST_REVCPU_REVFEATURE_H_

#include <string>
#include <cstdint>

// -- SST Headers
#include "SST.h"

namespace SST::RevCPU{

/// Table of RISC-V extension flags. They must be powers of two which can be
/// ORed to indicate multiple extensions being present.
enum RevFeatureType : uint32_t {
  RV_UNKNOWN  = 0,      ///< RevFeatureType: unknown feature
  RV_E        = 1<<0,   ///< RevFeatureType: E-extension
  RV_I        = 1<<1,   ///< RevFeatureType: I-extension
  RV_M        = 1<<2,   ///< RevFeatureType: M-extension
  RV_A        = 1<<3,   ///< RevFeatureType: A-extension
  RV_F        = 1<<4,   ///< RevFeatureType: F-extension
  RV_D        = 1<<5,   ///< RevFeatureType: D-extension
  RV_Q        = 1<<6,   ///< RevFeatureType: Q-extension
  RV_L        = 1<<7,   ///< RevFeatureType: L-extension
  RV_C        = 1<<8,   ///< RevFeatureType: C-extension
  RV_B        = 1<<9,   ///< RevFeatureType: B-extension
  RV_J        = 1<<10,  ///< RevFeatureType: J-extension
  RV_T        = 1<<11,  ///< RevFeatureType: T-extension
  RV_P        = 1<<12,  ///< RevFeatureType: P-Extension
  RV_V        = 1<<13,  ///< RevFeatureType: V-extension
  RV_N        = 1<<14,  ///< RevFeatureType: N-extension
  RV_ZICSR    = 1<<15,  ///< RevFEatureType: Zicsr-extension
  RV_ZIFENCEI = 1<<16,  ///< RevFeatureType: Zifencei-extension
  RV_ZAM      = 1<<17,  ///< RevFeatureType: Zam-extension
  RV_ZTSO     = 1<<18,  ///< RevFeatureType: Ztso-extension
  RV_ZFA      = 1<<19,  ///< RevFeatureType: Zfa-extension
};

class RevFeature{
public:
  /// RevFeature: standard constructor
  RevFeature( std::string Machine, SST::Output *Output,
              unsigned Min, unsigned Max, unsigned Id );

  /// RevFeature: standard destructor
  ~RevFeature() = default;

  /// RevFeature: deleted copy constructor
  RevFeature( const RevFeature& ) = delete;

  /// RevFeature: deleted copy assignment operator
  RevFeature& operator=( const RevFeature& ) = delete;

  /// IsModeEnabled: determines if the target mode is enabled
  bool IsModeEnabled( RevFeatureType Type ) const {
    return (features & Type) == Type;
  }

  /// SetMachineEntry: set the machine model item
  void SetMachineEntry( RevFeatureType Type ) {
    features = RevFeatureType{features | Type};
  }

  /// GetMachineModel: retreive the feature string
  auto GetMachineModel() const { return machine; }

  /// GetFeatures: retrieve the feature encoding
  auto GetFeatures() const { return features; }

  /// GetMinCost: get the minimum cost
  auto GetMinCost() const { return MinCost; }

  /// GetMaxCost: get the maximum cost
  auto GetMaxCost() const { return MaxCost; }

  /// IsRV32: Is the device an RV32
  bool IsRV32() const { return xlen == 32; }

  /// IsRV64: Is the device an RV64
  bool IsRV64() const { return xlen == 64; }

  /// HasF: Does the device support F?
  bool HasF() const { return IsModeEnabled(RV_F); }

  /// HasD: Does the device support D?
  bool HasD() const { return IsModeEnabled(RV_D); }

  /// HasCompressed: Returns whether RV32 or RV64 "C" is enabled
  bool HasCompressed() const { return IsModeEnabled(RV_C); }

  /// GetHart: Retrieve the hart of the target object
  auto GetHart() const { return Hart; }

private:
  std::string machine;      ///< RevFeature: feature string
  SST::Output *output;      ///< RevFeature: output handler
  unsigned MinCost;         ///< RevFeature: min memory cost
  unsigned MaxCost;         ///< RevFeature: max memory cost
  unsigned Hart;            ///< RevFeature: RISC-V CPU ID, aka "hart"
  RevFeatureType features;  ///< RevFeature: feature elements
  unsigned xlen;            ///< RevFeature: RISC-V Xlen

  /// ParseMachineModel: parse the machine model string
  bool ParseMachineModel();
}; // class RevFeature

} // namespace SST::RevCPU

#endif

// EOF
