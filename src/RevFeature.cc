//
// _RevFeature_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevFeature.h"
#include <array>
#include <utility>
#include <cstring>
#include <string_view>

using namespace SST::RevCPU;

RevFeature::RevFeature( std::string Machine,
                        SST::Output *Output,
                        unsigned Min,
                        unsigned Max,
                        unsigned Id )
  : machine(Machine), output(Output), MinCost(Min), MaxCost(Max), Hart(Id), features(RV_UNKNOWN), xlen(0) {
  output->verbose(CALL_INFO, 6, 0,
                  "Core %u ; Initializing feature set from machine string=%s\n",
                  Hart,
                  machine.c_str());
  if( !ParseMachineModel() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to parse the machine model: %s\n", Machine.c_str());
}

bool RevFeature::ParseMachineModel(){
  // walk the feature string
  const char* mac = machine.c_str();

  // -- step 1: parse the memory model
  if(!strncasecmp(mac, "RV32", 4))
    xlen = 32;
  else if(!strncasecmp(mac, "RV64", 4))
    xlen = 64;
  else
    return false;
  mac += 4;

  output->verbose(CALL_INFO, 6, 0, "Core %u ; Setting XLEN to %u\n", Hart, xlen);
  output->verbose(CALL_INFO, 6, 0, "Core %u ; Architecture string=%s\n", Hart, mac);

  using Entry = std::pair<std::string_view, unsigned>;
  static constexpr std::array table = {
    Entry { "E",          RV_E                                                      },
    Entry { "I",          RV_I                                                      },
    Entry { "M",          RV_M                                                      },
    Entry { "A",          RV_A                                                      },
    Entry { "F",          RV_F | RV_ZICSR                                           },
    Entry { "D",          RV_D | RV_F | RV_ZICSR                                    },
    Entry { "G",          RV_I | RV_M | RV_A | RV_F | RV_D | RV_ZICSR | RV_ZIFENCEI },
    Entry { "Q",          RV_Q | RV_D | RV_F | RV_ZICSR                             },
    Entry { "L",          RV_L                                                      },
    Entry { "C",          RV_C                                                      },
    Entry { "B",          RV_B                                                      },
    Entry { "J",          RV_J                                                      },
    Entry { "T",          RV_T                                                      },
    Entry { "P",          RV_P                                                      },
    Entry { "V",          RV_V | RV_D | RV_F | RV_ZICSR                             },
    Entry { "N",          RV_N                                                      },
    Entry { "Zicsr",      RV_ZICSR                                                  },
    Entry { "Zifencei",   RV_ZIFENCEI                                               },
    Entry { "Zam",        RV_ZAM | RV_A                                             },
    Entry { "Ztso",       RV_ZTSO                                                   },
    Entry { "Zfa",        RV_ZFA | RV_F | RV_ZICSR                                  },
  };

  // -- step 2: parse all the features
  // Note: Extension strings, if present, must appear in the order listed in the table above.
  if (*mac){
    for (const auto& tab : table) {
      // Look for an architecture string matching the current extension
      if(!strncasecmp(mac, tab.first.data(), tab.first.size())){

        // Set the machine entries for the matching extension
        SetMachineEntry(RevFeatureType{tab.second});

        // Move past the currently matching extension
        mac += tab.first.size();

        // Skip underscore separators
        while (*mac == '_') ++mac;

        // Success if end of string is reached
        if (!*mac)
          return true;
      }
    }
  }
  return false;
}

// EOF
