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
#include <cctype>
#include <cstring>
#include <string_view>
#include <utility>

namespace SST::RevCPU {

RevFeature::RevFeature( std::string Machine, SST::Output* Output, unsigned Min, unsigned Max, unsigned Id )
  : machine( std::move( Machine ) ), output( Output ), MinCost( Min ), MaxCost( Max ), ProcID( Id ), HartToExecID( 0 ),
    features( RV_UNKNOWN ), xlen( 0 ) {
  output->verbose( CALL_INFO, 6, 0, "Core %u ; Initializing feature set from machine string=%s\n", ProcID, machine.c_str() );
  if( !ParseMachineModel() )
    output->fatal( CALL_INFO, -1, "Error: failed to parse the machine model: %s\n", machine.c_str() );
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

  output->verbose( CALL_INFO, 6, 0, "Core %u ; Setting XLEN to %u\n", ProcID, xlen );
  output->verbose( CALL_INFO, 6, 0, "Core %u ; Architecture string=%s\n", ProcID, mac );

  // clang-format off
  ///< List of architecture extensions. These must listed in canonical order
  ///< as shown in Table 27.11, Chapter 27, of the RISC-V Unprivileged Spec
  ///< (Table 74 of Chapter 36 in the 2024 version).
  ///<
  ///< By using a canonical ordering, the extensions' presence can be tested
  ///< in linear time complexity of the table and the string. Some of the
  ///< extensions imply other extensions, so the extension flags are ORed.
  ///<
  ///< The second and third values are the major and minor default version.
  ///< The fourth and fifth values are the major version range that Rev supports.
  ///< Values of -1, 0 for the fourth and fifth values indicates no Rev support yet.
  ///<
  ///< ExtensionName DefaultMajor DefaultMinor MinSupportedVersion MaxSupportedVersion Flags
  static constexpr std::tuple<std::string_view, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t> table[] = {
    { "I",          2, 1,  2, 2, RV_I                                   },
    { "E",          2, 0, -1, 0, RV_E                                   }, // Unsupported
    { "M",          2, 0,  2, 2, RV_M | RV_ZMMUL                        },
    { "A",          2, 1,  2, 2, RV_A                                   },
    { "F",          2, 2,  2, 2, RV_F | RV_ZICSR                        },
    { "D",          2, 2,  2, 2, RV_D | RV_F | RV_ZICSR                 },
    { "G",          2, 0,  2, 2, RV_I | RV_M | RV_ZMMUL | RV_A |
                                 RV_F | RV_D | RV_ZICSR | RV_ZIFENCEI   },
    { "Q",          2, 2, -1, 0, RV_Q | RV_D | RV_F | RV_ZICSR          }, // Unsupported
    { "C",          2, 0,  2, 2, RV_C                                   },
    { "B",          1, 0,  1, 0, RV_ZBA | RV_ZBB | RV_ZBS               },
    { "P",          0, 2, -1, 0, RV_P                                   }, // Unsupported
    { "V",          1, 0, -1, 0, RV_V | RV_D | RV_F | RV_ZICSR          },
    { "H",          1, 0, -1, 0, RV_H                                   }, // Unsupported
    { "Zicbom",     1, 0,  1, 1, RV_ZICBOM                              },
    { "Zicsr",      2, 0,  2, 2, RV_ZICSR                               },
    { "Zifencei",   2, 0,  2, 2, RV_ZIFENCEI                            },
    { "Zmmul",      1, 0,  1, 1, RV_ZMMUL                               },
    { "Zfa",        1, 0,  1, 1, RV_ZFA | RV_F | RV_ZICSR               },
    { "Zfh",        1, 0, -1, 0, RV_ZFH | RV_ZFHMIN | RV_F | RV_ZICSR   }, // Unsupported
    { "Zfhmin",     1, 0, -1, 0, RV_ZFHMIN | RV_F | RV_ZICSR            }, // Unsupported
    { "Zba",        1, 0,  1, 0, RV_ZBA                                 },
    { "Zbb",        1, 0,  1, 0, RV_ZBB                                 },
    { "Zbc",        1, 0,  1, 0, RV_ZBC                                 },
    { "Zbs",        1, 0,  1, 0, RV_ZBS                                 },
    { "Ztso",       1, 0, -1, 0, RV_ZTSO                                }, // Unsupported
  };
  // clang-format on

  // -- step 2: parse all the features
  // Note: Extension strings, if present, must appear in the order listed in the table above.
  if( *mac ) {
    char unsupported_version[128];
    *unsupported_version = 0;

    for( auto [ext, majorVersion, minorVersion, minimumVersion, maximumVersion, flags] : table ) {
      // Look for an architecture string matching the current extension
      if( !strncasecmp( mac, ext.data(), ext.size() ) ) {

        // Set the machine entries for the matching extension
        SetMachineEntry( RevFeatureType{ flags } );

        // Move past the currently matching extension
        mac += ext.size();

        // Optional version string follows extension
        if( isdigit( *mac ) ) {
          majorVersion = strtoul( mac, const_cast<char**>( &mac ), 10 );
          if( tolower( *mac ) == 'p' && isdigit( *++mac ) )
            minorVersion = strtoul( mac, const_cast<char**>( &mac ), 10 );
        }

        // Record first unsupported extension version
        // Error is delayed so that parse errors in the architecture string take priority
        if( ( majorVersion < minimumVersion || majorVersion > maximumVersion ) && !*unsupported_version ) {
          snprintf(
            unsupported_version,
            sizeof( unsupported_version ),
            "Error: Version %" PRIu32 ".%" PRIu32 " of %s extension is not supported\n",
            majorVersion,
            minorVersion,
            ext.data()
          );
        }

        // Skip underscore separators
        while( *mac == '_' )
          ++mac;

        // Success if end of string is reached
        if( !*mac ) {
          // Report error on first unsupported extension version
          if( *unsupported_version ) {
            output->fatal( CALL_INFO, -1, "%s", unsupported_version );
          }
          return true;
        }
      }
    }
  }
  return false;
}

}  // namespace SST::RevCPU

// EOF
