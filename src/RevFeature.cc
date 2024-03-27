//
// _RevFeature_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevFeature.h"
#include <cstring>
#include <string_view>
#include <utility>

namespace SST::RevCPU {

RevFeature::RevFeature( std::string  Machine,
                        SST::Output* Output,
                        unsigned     Min,
                        unsigned     Max,
                        unsigned     Id ) :
  machine( std::move( Machine ) ),
  output( Output ), MinCost( Min ), MaxCost( Max ), ProcID( Id ),
  HartToExecID( 0 ), features( RV_UNKNOWN ), xlen( 0 ) {
  output->verbose(
    CALL_INFO,
    6,
    0,
    "Core %u ; Initializing feature set from machine string=%s\n",
    ProcID,
    machine.c_str() );
  if( !ParseMachineModel() )
    output->fatal( CALL_INFO,
                   -1,
                   "Error: failed to parse the machine model: %s\n",
                   machine.c_str() );
}

bool RevFeature::ParseMachineModel() {
  // walk the feature string
  const char* mac = machine.c_str();

  // -- step 1: parse the memory model
  if( !strncasecmp( mac, "RV32", 4 ) )
    xlen = 32;
  else if( !strncasecmp( mac, "RV64", 4 ) )
    xlen = 64;
  else
    return false;
  mac += 4;

  output->verbose(
    CALL_INFO, 6, 0, "Core %u ; Setting XLEN to %u\n", ProcID, xlen );
  output->verbose(
    CALL_INFO, 6, 0, "Core %u ; Architecture string=%s\n", ProcID, mac );

  ///< List of architecture extensions. These must listed in canonical order
  ///< as shown in Table 27.11, Chapter 27, of the RISC-V Unprivileged Spec
  ///< (Table 74 of Chapter 36 in the 2024 version).
  ///<
  ///< By using a canonical ordering, the extensions' presence can be tested
  ///< in linear time complexity of the table and the string. Some of the
  ///< extensions imply other extensions, so the extension flags are ORed.
  // clang-format off
  static constexpr std::pair<std::string_view, uint32_t> table[] = {
    { "I",          RV_I                                                      },
    { "E",          RV_E                                                      },
    { "M",          RV_M                                                      },
    { "A",          RV_A                                                      },
    { "F",          RV_F | RV_ZICSR                                           },
    { "D",          RV_D | RV_F | RV_ZICSR                                    },
    { "G",          RV_I | RV_M | RV_A | RV_F | RV_D | RV_ZICSR | RV_ZIFENCEI },
    { "Q",          RV_Q | RV_D | RV_F | RV_ZICSR                             },
    { "C",          RV_C                                                      },
    { "P",          RV_P                                                      },
    { "V",          RV_V | RV_D | RV_F | RV_ZICSR                             },
    { "H",          RV_H                                                      },
    { "Zicsr",      RV_ZICSR                                                  },
    { "Zifencei",   RV_ZIFENCEI                                               },
    { "Ztso",       RV_ZTSO                                                   },
    { "Zfa",        RV_ZFA | RV_F | RV_ZICSR                                  },
    { "Zicbom",     RV_ZICBOM                                                 },
  };
  // clang-format on

  // -- step 2: parse all the features
  // Note: Extension strings, if present, must appear in the order listed in the table above.
  if( *mac ) {
    for( const auto& [ext, flags] : table ) {
      // Look for an architecture string matching the current extension
      if( !strncasecmp( mac, ext.data(), ext.size() ) ) {

        // Set the machine entries for the matching extension
        SetMachineEntry( RevFeatureType{ flags } );

        // Move past the currently matching extension
        mac += ext.size();

        // Skip underscore separators
        while( *mac == '_' )
          ++mac;

        // Success if end of string is reached
        if( !*mac )
          return true;
      }
    }
  }
  return false;
}

}  // namespace SST::RevCPU

// EOF
