//
// _RevFeature_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVFEATURE_H_
#define _SST_REVCPU_REVFEATURE_H_

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>

// -- Standard Headers
#include <cinttypes>
#include <string>

namespace SST::RevCPU {
  class RevFeature;
}

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{

    typedef enum{
      RV_UNKNOWN    = 0,        ///< RevFeatureType: unknown feature
      RV_I          = 1,        ///< RevFeatureType: I-extension
      RV_M          = 2,        ///< RevFeatureType: M-extension
      RV_A          = 3,        ///< RevFeatureType: A-extension
      RV_F          = 4,        ///< RevFeatureType: F-extension
      RV_D          = 5,        ///< RevFeatureType: D-extension
      RV_C          = 6,        ///< RevFeatureType: C-extension
      RV_P          = 20        ///< RevFeatureType: PAN Extension
    }RevFeatureType;

    class RevFeature{
    public:
      /// RevFeature: standard constructor
      RevFeature( std::string Machine, SST::Output *Output,
                  unsigned Min, unsigned Max, unsigned Id );

      /// RevFeature: standard desctructor
      ~RevFeature();

      /// RevFeature: determines if the target mdoe is enabled
      bool IsModeEnabled( RevFeatureType Type );

      /// RevFeature: retreive the feature string
      std::string GetMachineModel() { return machine; }

      /// RevFeature: retrieve the feature encoding
      uint64_t GetFeatures() { return features; }

      /// RevFeature: retrieve the xlen
      unsigned GetXlen() { return xlen; }

      /// RevFeature: get the minimum cost
      unsigned GetMinCost() { return MinCost; }

      /// RevFeature: get the maximum cost
      unsigned GetMaxCost() { return MaxCost; }

      /// RevFeature: Is the device an RV32
      bool IsRV32() { if( xlen == 32 ){ return true; }return false; }

      /// RevFeature: Is the device an RV64
      bool IsRV64() { if( xlen == 64 ){ return true; }return false; }

      /// RevFeature: Does the device support RV32C?
      bool IsRV32C();

      /// RevFeature: Does the device support RV32F?
      bool IsRV32F();

      /// RevFeature: Does the device support RV64F?
      bool IsRV64F();

      /// RevFeature: Does the device support RV32D?
      bool IsRV32D();

      /// RevFeature: Does the device support RV64D?
      bool IsRV64D();

      /// RevFeature: Retrieve the hart of the target object
      unsigned GetHart() { return Hart; }

    private:
      std::string machine;      ///< RevFeature: feature string
      SST::Output *output;      ///< RevFeature: output handler
      unsigned MinCost;         ///< RevFeature: min memory cost
      unsigned MaxCost;         ///< RevFeature: max memory cost
      unsigned Hart;            ///< RevFeature: RISC-V CPU ID, aka "hart"
      uint64_t features;        ///< RevFeature: feature elements
      unsigned xlen;            ///< RevFeature: RISC-V xlen

      /// RevFeature: parse the machine model string
      bool ParseMachineModel();

      /// RevFeature: set the machine model item
      void SetMachineEntry( RevFeatureType Type );
    }; // class RevFeature
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
