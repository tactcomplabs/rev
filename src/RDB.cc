
//
// _RDB_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RDB.h"


namespace SST::RevCPU{
  RDB::RDB(SST::Cycle_t firstBreak):
    breakCycle(firstBreak), breakPC(0), proc(nullptr), cmdLine() {

    cmdLine.clear();
  }

  RDB::~RDB(){

  }

  void RDB::PrintHelp(){
    std::cout << "Supported commands:" << std::endl;
    std::cout << "\treg <hart> <num>  // Print Register value. num >= 32 will print all regs"  << std::endl;
    std::cout << "\ts <num>           // Step forward num cycles - default is one cycle"       << std::endl;
    std::cout << "\tpc <hart> <num>   // Execute until hart reaches pc == num"                 << std::endl;
    std::cout << "\tc                 // Continue (exit debugger)"                             << std::endl;
  }

  bool RDB::GetCommand(){

    //Supported commands:
    // reg <hart> <num>  // Print Register value
    // s <num>           // Step forward num cycles - default is one cycle
    // pc <hart> <num>   // Execute until hart reaches pc == num. num must be in hex
    // c                 // Continue (exit debugger)
    
    std::string input;
    std::stringstream line;
    std::string field;
    bool rtn = true;
    std::cout << "Core " << proc->id << " PC: " << std::hex << proc->GetPC() << std::dec << " rdb% ";
    std::getline(std::cin, input);
    line.str(input);
    for(field; std::getline(line, field, ' ');){
      cmdLine.push_back(field);
    }

    if(cmdLine[0] == "reg"){
      PrintRegister();
      rtn = true;
    }else if(cmdLine[0] == "s"){
      Step();
      rtn = false;
    }else if(cmdLine[0] == "pc"){
      PC();
      rtn = true;
    }else if(cmdLine[0] == "c"){
      breakCycle = 0;
      rtn = false;
    }else{
      std::cout << "INVALID COMMAND" << std::endl;
      PrintHelp();
      rtn = true;
    }

    cmdLine.clear();
    return rtn;
  }

  void RDB::Step(){
    int stepCount = 1;
    if(cmdLine.size() > 1){
      stepCount = std::stoi(cmdLine[1]);
    }
    breakCycle += stepCount;
  }

  void RDB::PrintRegister(){
    uint64_t val = 0;
    int reg = std::stoi(cmdLine[2]);
    int hart = std::stoi(cmdLine[1]);
    if(reg < _REV_NUM_REGS_){
      proc->DebugReadReg(reg, &val, hart);
      std::cout << "Core " << proc->id << ":" << hart << " x" << reg << " = " << std::hex << val << std::dec << std::endl;
    }else{
      std::cout << "Core " << proc->id << " x" << reg << " is out of range" << std::endl;
      if(proc->RegFile) {std::cout << *(proc->RegFile);}
    }
  }

  void RDB::PC(){
    breakPC = std::stoi(cmdLine[2], nullptr, 16);
  }

} //namespace
