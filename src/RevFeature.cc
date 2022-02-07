//
// _RevFeature_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevFeature.h"

RevFeature::RevFeature( std::string Machine,
                        SST::Output *Output,
                        unsigned Min,
                        unsigned Max,
                        unsigned Id )
  : machine(Machine), output(Output),
    MinCost(Min), MaxCost(Max), Hart(Id),
    features(0x00ull), xlen(64) {
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Initializing feature set from machine string=%s\n",
                    Hart,
                    machine.c_str());
  if( !ParseMachineModel() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to parse the machine model: %s\n", Machine.c_str());
}

RevFeature::~RevFeature(){
}

bool RevFeature::IsRV32C(){
  if( IsRV32() && !IsRV64() && IsModeEnabled(RV_C) )
    return true;
  return false;
}

bool RevFeature::IsRV32F(){
  if( IsRV32() && IsModeEnabled(RV_F) )
    return true;
  return false;
}

bool RevFeature::IsRV64F(){
  if( IsRV64() && IsModeEnabled(RV_F) )
    return true;
  return false;
}

bool RevFeature::IsRV32D(){
  if( IsRV32() && IsModeEnabled(RV_D) )
    return true;
  return false;
}

bool RevFeature::IsRV64D(){
  if( IsRV64() && IsModeEnabled(RV_D) )
    return true;
  return false;
}

bool RevFeature::IsModeEnabled( RevFeatureType Type ){
  uint64_t mask = 0x01ull << Type;
  if( (features & mask) > 0x00ull )
    return true;

  return false;
}

void RevFeature::SetMachineEntry( RevFeatureType Type ){
  features |= (0x01ull << Type);
}

bool RevFeature::ParseMachineModel(){
  if( machine.length() < 4 )
    return false;

  // walk the feature string

  // -- step 1: parse the memory model
  std::size_t found = machine.find("RV32",0);
  if( found != std::string::npos)
    xlen = 32;

  found = machine.find("RV64",0);
  if( found != std::string::npos )
    xlen = 64;

  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Setting XLEN to %d\n",
                    Hart,
                    xlen);

  found += 5;
  std::string arch = machine.substr(4,std::string::npos);
  output->verbose(CALL_INFO,6,0,"Core %d ; Architecture string=%s\n", Hart, arch.c_str());
  found = 0;

  // -- step 2: parse all the features
  while( found < arch.length() ){
    switch( arch[found] ){
    case 'I':
      SetMachineEntry(RV_I);
      break;
    case 'M':
      SetMachineEntry(RV_M);
      break;
    case 'A':
      SetMachineEntry(RV_A);
      break;
    case 'F':
      SetMachineEntry(RV_F);
      break;
    case 'D':
      SetMachineEntry(RV_D);
      break;
    case 'G':
      xlen = 64;
      SetMachineEntry(RV_I);
      SetMachineEntry(RV_M);
      SetMachineEntry(RV_A);
      SetMachineEntry(RV_F);
      SetMachineEntry(RV_D);
      break;
    case 'C':
      SetMachineEntry(RV_C);
      break;
    case 'P':
      SetMachineEntry(RV_P);
      break;
    default:
    case 'E':
    case 'Q':
    case 'L':
    case 'B':
    case 'J':
    case 'T':
    case 'V':
    case 'N':
    case 'H':
    case 'S':
      return false;
      break;
    }
    found = found + 1;
  }

  return true;
}

// EOF
