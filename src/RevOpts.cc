//
// _RevOpts_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevOpts.h"

RevOpts::RevOpts( unsigned NumCores, const int Verbosity )
  : numCores(NumCores), verbosity(Verbosity) {

  std::pair<unsigned,unsigned> InitialPair;
  InitialPair.first = 0;
  InitialPair.second = 10;

  // init all the standard options
  // -- startAddr = 0x00000000
  // -- machine = "G" aka, "IMAFD"
  // -- pipeLine = 5
  // -- table = internal
  // -- memCosts[core] = 0:10
  // -- prefetch depth = 16
  for( unsigned i=0; i<numCores; i++ ){
    startAddr.insert( std::pair<unsigned,uint64_t>(i,(uint64_t)(0x00000000)) );
    machine.insert( std::pair<unsigned,std::string>(i,"G") );
    table.insert( std::pair<unsigned,std::string>(i,"_REV_INTERNAL_") );
    memCosts.push_back(InitialPair);
    prefetchDepth.insert( std::pair<unsigned,unsigned>(i,16) );
  }
}

RevOpts::~RevOpts(){
}

void RevOpts::splitStr(const std::string& s,
                       char c,
                       std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);
  v.clear();

  if( (j==std::string::npos) && (s.length() > 0) ){
    v.push_back(s);
    return ;
  }

  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }
}

bool RevOpts::InitPrefetchDepth( std::vector<std::string> Depths ){
  std::vector<std::string> vstr;
  for(unsigned i=0; i<Depths.size(); i++ ){
    std::string s = Depths[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    if( Core > numCores )
      return false;

    std::string::size_type sz = 0;
    unsigned Depth = (unsigned)(std::stoul(vstr[1],&sz,0));

    prefetchDepth.find(Core)->second = Depth;
    vstr.clear();
  }
  return true;
}

bool RevOpts::InitStartAddrs( std::vector<std::string> StartAddrs ){
  std::vector<std::string> vstr;

  // check to see if we expand into multiple cores
  if( StartAddrs.size() == 1 ){
    std::string s = StartAddrs[0];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    if( vstr[0] == "CORES" ){
      // set all cores to the target machine model
      std::string::size_type sz = 0;
      uint64_t Addr = (uint64_t)(std::stoull(vstr[1],&sz,0));
      for( unsigned i=0; i<numCores; i++ ){
        startAddr.find(i)->second = Addr;
      }
      return true;
    }
  }

  for(unsigned i=0; i<StartAddrs.size(); i++ ){
    std::string s = StartAddrs[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    if( Core > numCores )
      return false;

    std::string::size_type sz = 0;
    uint64_t Addr = (uint64_t)(std::stoull(vstr[1],&sz,0));

    startAddr.find(Core)->second = Addr;
    vstr.clear();
  }
  return true;
}

bool RevOpts::InitStartSymbols( std::vector<std::string> StartSymbols ){
  std::vector<std::string> vstr;
  for(unsigned i=0; i<StartSymbols.size(); i++ ){
    std::string s = StartSymbols[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    if( Core > numCores )
      return false;

    startSym.insert(std::pair<unsigned,std::string>(Core,vstr[1]));
    vstr.clear();
  }
  return true;
}

bool RevOpts::InitMachineModels( std::vector<std::string> Machines ){
  std::vector<std::string> vstr;

  // check to see if we expand into multiple cores
  if( Machines.size() == 1 ){
    std::string s = Machines[0];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    if( vstr[0] == "CORES" ){
      // set all cores to the target machine model
      for( unsigned i=0; i<numCores; i++ ){
        machine.at(i) = vstr[1];
      }
      return true;
    }
  }

  // parse individual core configs
  for( unsigned i=0; i<Machines.size(); i++ ){
    std::string s = Machines[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    if( Core > numCores )
      return false;

    machine.at(Core) = vstr[1];
    vstr.clear();
  }
  return true;
}

bool RevOpts::InitInstTables( std::vector<std::string> InstTables ){
  std::vector<std::string> vstr;
  for( unsigned i=0; i<InstTables.size(); i++ ){
    std::string s = InstTables[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 2 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    if( Core > numCores )
      return false;

    table.at(Core) = vstr[1];
    vstr.clear();
  }
  return true;
}

bool RevOpts::InitMemCosts( std::vector<std::string> MemCosts ){
  std::vector<std::string> vstr;

  for( unsigned i=0; i<MemCosts.size(); i++ ){
    std::string s = MemCosts[i];
    splitStr(s,':',vstr);
    if( vstr.size() != 3 )
      return false;

    unsigned Core = (unsigned)(std::stoi(vstr[0],nullptr,0));
    unsigned Min  = (unsigned)(std::stoi(vstr[1],nullptr,0));
    unsigned Max  = (unsigned)(std::stoi(vstr[2],nullptr,0));
    memCosts[Core].first  = Min;
    memCosts[Core].second = Max;
    if( (Min==0) || (Max==0) ){
      return false;
    }
    vstr.clear();
  }

  return true;
}

bool RevOpts::GetPrefetchDepth( unsigned Core, unsigned &Depth ){
  if( Core > numCores )
    return false;

  if( prefetchDepth.find(Core) == prefetchDepth.end() )
    return false;

  Depth = prefetchDepth.at(Core);
  return true;
}

bool RevOpts::GetStartAddr( unsigned Core, uint64_t &StartAddr ){
  if( Core > numCores )
    return false;

  if( startAddr.find(Core) == startAddr.end() )
    return false;

  StartAddr = startAddr.at(Core);
  return true;
}

bool RevOpts::GetStartSymbol( unsigned Core, std::string &Symbol ){
  if( Core > numCores )
    return false;

  if( startSym.find(Core) == startSym.end() ){
    Symbol = "main";
    //Symbol = "_start";
  }else{
    Symbol = startSym.at(Core);
  }
  return true;
}

bool RevOpts::GetMachineModel( unsigned Core, std::string &MachModel ){
  if( Core > numCores )
    return false;

  MachModel = machine.at(Core);
  return true;
}

bool RevOpts::GetInstTable( unsigned Core, std::string &Table ){
  if( Core > numCores )
    return false;

  Table = table.at(Core);
  return true;
}

bool RevOpts::GetMemCost( unsigned Core, unsigned &Min, unsigned &Max ){
  if( Core > numCores )
    return false;

  Min = memCosts[Core].first;
  Max = memCosts[Core].second;

  return true;
}

// EOF
