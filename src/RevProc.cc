//
// _RevProc_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#define _DEFAULT_THREAD_MEM_SIZE_ 4*1024*1024 // 4 MB

#include "RevInstTable.h"

#include "../include/RevProc.h"
#include <bitset>
#include <cstdint>
#include <optional>
#include <utility>
#include <filesystem>

RevProc::RevProc( unsigned Id,
                  RevOpts *Opts,
                  RevMem *Mem,
                  RevLoader *Loader,
                  SST::Output *Output )
  : Halted(false), Stalled(false), SingleStep(false),
    CrackFault(false), ALUFault(false), fault_width(0),
    id(Id), HartToDecode(0), HartToExec(0), Retired(0x00ull),
    opts(Opts), mem(Mem), loader(Loader), output(Output),
    feature(nullptr), PExec(nullptr), sfetch(nullptr) {

  // initialize the machine model for the target core
  std::string Machine;
  if( !Opts->GetMachineModel(id,Machine) )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve the machine model for core=%d\n", id);

  unsigned MinCost = 0;
  unsigned MaxCost = 0;

  Opts->GetMemCost(Id,MinCost,MaxCost);

  feature = new RevFeature(Machine,output,MinCost,MaxCost,Id);
  if( !feature )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevFeature object for core=%d\n", id);

  unsigned Depth = 0;
  Opts->GetPrefetchDepth(Id, Depth);
  if( Depth == 0 ){
    Depth = 16;
  }

  sfetch = new RevPrefetcher(Mem,Depth);
  if( !sfetch )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevPrefetcher object for core=%d\n", id);

  // Initialize ThreadTable (NOTE: Default PID = 1024 + ProcID)
  if( !InitThreadTable() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to initialize the ThreadTable for core=%d\n", id );

  std::cout << "ThreadTable.size()" << ThreadTable.size() << std::endl;
  // load the instruction tables
  if( !LoadInstructionTable() )
    output->fatal(CALL_INFO, -1,
                  "Error : failed to load instruction table for core=%d\n", id );

  // Initialize EcallTable
  InitEcallTable();
  if( Ecalls.size() <= 0 )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to initialize the Ecall Table for core=%d\n", id );

  // reset the core
  if( !Reset() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to reset the core resources for core=%d\n", id );

  Stats.totalCycles = 0;
  Stats.cyclesBusy = 0;
  Stats.cyclesIdle_Total = 0;
  Stats.cyclesIdle_Pipeline = 0;
  Stats.cyclesIdle_MemoryFetch= 0;
  Stats.cyclesStalled = 0;
  Stats.percentEff = 0.0;
  Stats.floatsExec = 0;
}

RevProc::~RevProc(){
  for( unsigned i=0; i<Extensions.size(); i++ )
    delete Extensions[i];
  delete feature;
  delete sfetch;
}

RevProc::RevProcStats RevProc::GetStats(){
  Stats.memStats.bytesRead      = mem->memStats.bytesRead;
  Stats.memStats.bytesWritten   = mem->memStats.bytesWritten;
  Stats.memStats.doublesRead    = mem->memStats.doublesRead;
  Stats.memStats.doublesWritten = mem->memStats.doublesWritten;
  Stats.memStats.floatsRead     = mem->memStats.floatsRead;
  Stats.memStats.floatsWritten  = mem->memStats.floatsWritten;
  return Stats;
}

bool RevProc::Halt(){
  if( Halted )
    return false;
  Halted = true;
  SingleStep = false;
  return true;
}

bool RevProc::Resume(){
  if( Halted ){
    Halted = false;
    SingleStep = false;
    return true;
  }
  return false;
}

bool RevProc::SingleStepHart(){
  if( SingleStep )
    return true;
  if( Halted ){
    Halted = false;
    SingleStep = true;
    return true;
  }else{
    // must be halted to single step
    return false;
  }
}

bool RevProc::EnableExt(RevExt *Ext, bool Opt){
  if( !Ext )
    output->fatal(CALL_INFO, -1, "Error: failed to initialize RISC-V extensions\n");

  output->verbose(CALL_INFO, 6, 0,
                  "Core %d ; Enabling extension=%s\n",
                  id, Ext->GetName().c_str());

  // add the extension to our vector of enabled objects
  Extensions.push_back(Ext);

  // retrieve all the target instructions
  std::vector<RevInstEntry> IT = Ext->GetInstTable();

  // setup the mapping of InstTable to Ext objects
  InstTable.reserve(InstTable.size() + IT.size());

  for( unsigned i=0; i<IT.size(); i++ ){
    InstTable.push_back(IT[i]);
    std::pair<unsigned,unsigned> ExtObj =
      std::pair<unsigned,unsigned>(Extensions.size()-1,i);
    EntryToExt.insert(
      std::pair<unsigned,
        std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
  }

  // load the compressed instructions
  if( feature->IsModeEnabled(RV_C) ){
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Enabling compressed extension=%s\n",
                    id, Ext->GetName().c_str());

    std::vector<RevInstEntry> CT = Ext->GetCInstTable();
    InstTable.reserve(InstTable.size() + CT.size());

    for( unsigned i=0; i<CT.size(); i++ ){
      InstTable.push_back(CT[i]);
      std::pair<unsigned,unsigned> ExtObj =
        std::pair<unsigned,unsigned>(Extensions.size()-1,i);
      EntryToExt.insert(
        std::pair<unsigned,
          std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
    }
    // load the optional compressed instructions
    if( Opt ){
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Enabling optional compressed extension=%s\n",
                      id, Ext->GetName().c_str());
      CT = Ext->GetOInstTable();

      InstTable.reserve(InstTable.size() + CT.size());

      for( unsigned i=0; i<CT.size(); i++ ){
        InstTable.push_back(CT[i]);
        std::pair<unsigned,unsigned> ExtObj =
          std::pair<unsigned,unsigned>(Extensions.size()-1,i);
        EntryToExt.insert(
          std::pair<unsigned,
            std::pair<unsigned,unsigned>>(InstTable.size()-1,ExtObj));
      }
    }
  }

  return true;
}

bool RevProc::SeedInstTable(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Seeding instruction table for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  // I-Extension
  if( feature->IsModeEnabled(RV_I) ){
    if( feature->GetXlen() == 64 ){
      // load RV32I & RV64; no optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile(),mem,output)),false);
      EnableExt(static_cast<RevExt *>(new RV64I(feature,RegFile(),mem,output)),false);
    }else{
      // load RV32I w/ optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile(0),mem,output)),true);
    }
  }

  // M-Extension
  if( feature->IsModeEnabled(RV_M) ){
    EnableExt(static_cast<RevExt *>(new RV32M(feature,RegFile(0),mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64M(feature,RegFile(0),mem,output)),false);
    }
  }

  // A-Extension
  if( feature->IsModeEnabled(RV_A) ){
    EnableExt(static_cast<RevExt *>(new RV32A(feature,RegFile(0),mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64A(feature,RegFile(0),mem,output)),false);
    }
  }

  // F-Extension
  if( feature->IsModeEnabled(RV_F) ){
    if( (!feature->IsModeEnabled(RV_D)) && (feature->GetXlen() == 32) ){
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile(0),mem,output)),true);
    }else{
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile(0),mem,output)),false);
    }
#if 0
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile(0),mem,output)));
    }
#endif
  }

  // D-Extension
  if( feature->IsModeEnabled(RV_D) ){
    EnableExt(static_cast<RevExt *>(new RV32D(feature,RegFile(0),mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile(0),mem,output)),false);
    }
  }

  // PAN Extension
  if( feature->IsModeEnabled(RV_P) ){
    EnableExt(static_cast<RevExt *>(new RV64P(feature,RegFile(0),mem,output)),false);
  }

  return true;
}

uint32_t RevProc::CompressCEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= (uint32_t)(Entry.opcode);
  Value |= (uint32_t)((uint32_t)(Entry.funct2)<<2);
  Value |= (uint32_t)((uint32_t)(Entry.funct3)<<4);
  Value |= (uint32_t)((uint32_t)(Entry.funct4)<<8);
  Value |= (uint32_t)((uint32_t)(Entry.funct6)<<12);

  return Value;
}

uint32_t RevProc::CompressEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= (uint32_t)(Entry.opcode);
  Value |= (uint32_t)((uint32_t)(Entry.funct3)<<8);
  Value |= (uint32_t)((uint32_t)(Entry.funct7)<<11);
  Value |= (uint32_t)((uint32_t)(Entry.imm12)<<18);

  return Value;
}

void RevProc::splitStr(const std::string& s,
                       char c,
                       std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);

  // catch strings with no delims
  if( j == std::string::npos ){
    v.push_back(s);
  }

  // break up the rest of the string
  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }
}

std::string RevProc::ExtractMnemonic(RevInstEntry Entry){
  std::string Tmp = Entry.mnemonic;
  std::vector<std::string> vstr;
  splitStr(Tmp,' ',vstr);

  return vstr[0];
}

bool RevProc::InitTableMapping(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Initializing table mapping for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  for( unsigned i=0; i<InstTable.size(); i++ ){
    NameToEntry.insert(
      std::pair<std::string,unsigned>(ExtractMnemonic(InstTable[i]),i) );
    if( !InstTable[i].compressed ){
      // map normal instruction
      EncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Table Entry %d = %s\n",
                      id,
                      CompressEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }else{
      // map compressed instruction
      CEncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressCEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; Compressed Table Entry %d = %s\n",
                      id,
                      CompressCEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }
  }
  return true;
}

bool RevProc::ReadOverrideTables(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Reading override tables for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  std::string Table;
  if( !opts->GetInstTable(id, Table) )
    return false;

  // if the length of the file name is 0, just return
  if( Table == "_REV_INTERNAL_" )
    return true;

  // open the file
  std::ifstream infile(Table);
  if( !infile.is_open() )
    output->fatal(CALL_INFO, -1, "Error: failed to read instruction table for core=%d\n", id);

  // read all the values
  std::string Inst;
  std::string Cost;
  unsigned Entry;
  std::map<std::string,unsigned>::iterator it;
  while( infile >> Inst >> Cost ){
    it = NameToEntry.find(Inst);
    if( it == NameToEntry.end() )
      output->fatal(CALL_INFO, -1, "Error: could not find instruction in table for map value=%s\n", Inst.c_str() );

    Entry = it->second;
    InstTable[Entry].cost = (unsigned)(std::stoi(Cost,nullptr,0));
  }

  // close the file
  infile.close();

  return true;
}

bool RevProc::LoadInstructionTable(){
  // Stage 1: load the instruction table for each enable feature
  if( !SeedInstTable() )
    return false;

  // Stage 2: setup the internal mapping tables for performance
  if( !InitTableMapping() )
    return false;

  // Stage 3: examine the user-defined cost tables to see if we need to override the defaults
  if( !ReadOverrideTables() )
    return false;

  return true;
}

bool RevProc::Reset(){
  // reset the register file
  for (int t=0;  t < _REV_HART_COUNT_; t++){
    RegFile(t)->RV32_PC = 0x00l;
    RegFile(t)->RV64_PC = 0x00ull;
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      RegFile(t)->RV32[i] = 0x00l;
      RegFile(t)->RV64[i] = 0x00ull;
      RegFile(t)->SPF[i]  = 0.f;
      RegFile(t)->DPF[i]  = 0.f;
      RegFile(t)->RV32_Scoreboard[i] = false;
      RegFile(t)->RV64_Scoreboard[i] = false;
      RegFile(t)->SPF_Scoreboard[i] = false;
      RegFile(t)->DPF_Scoreboard[i] = false;
    }

    // initialize all the relevant program registers
    // -- x2 : stack pointer
    RegFile(t)->RV32[2] = (uint32_t)(mem->GetStackTop());
    RegFile(t)->RV64[2] = mem->GetStackTop();

    // -- x3 : global pointer
    RegFile(t)->RV32[3] = (uint32_t)(loader->GetSymbolAddr("__global_pointer$"));
    RegFile(t)->RV64[3] = loader->GetSymbolAddr("__global_pointer$");

    // -- x8 : frame pointer
    RegFile(t)->RV32[8] = RegFile(t)->RV32[3];
    RegFile(t)->RV64[8] = RegFile(t)->RV64[3];

    RegFile(t)->cost = 0;

    while(!Pipeline.empty()){
      Pipeline.pop();
    }

  }
  // set the pc
  uint64_t StartAddr = 0x00ull;
  if( !opts->GetStartAddr( id, StartAddr ) )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to init the start address for core=%d\n", id);
  std::string StartSymbol = "main";
  if( StartAddr == 0x00ull ){
    if( !opts->GetStartSymbol( id, StartSymbol ) )
      output->fatal(CALL_INFO, -1,
                    "Error: failed to init the start symbol address for core=%d\n", id);

    StartAddr = loader->GetSymbolAddr(StartSymbol);
  }
  if( StartAddr == 0x00ull ){
    // load "main" symbol
    StartAddr = loader->GetSymbolAddr("main");
    if( StartAddr == 0x00ull ){
      output->fatal(CALL_INFO, -1,
                    "Error: failed to auto discover address for <main> for core=%d\n", id);
    }
  }
  for (int t=0;  t < _REV_HART_COUNT_; t++){
    RegFile(t)->RV32_PC = (uint32_t)(StartAddr);
    RegFile(t)->RV64_PC = StartAddr;
  }
  HART_CTS.set();

  return true;
}

bool RevProc::IsFloat(unsigned Entry){
  if( (InstTable[Entry].rdClass == RegFLOAT) ||
      (InstTable[Entry].rs1Class == RegFLOAT) ||
      (InstTable[Entry].rs2Class == RegFLOAT) ||
      (InstTable[Entry].rs3Class == RegFLOAT) ){
    return true;
  }
  return false;
}

RevInst RevProc::DecodeCRInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct4  = InstTable[Entry].funct4;

  // registers
  CompInst.rd      = DECODE_RD(Inst);
  CompInst.rs1     = DECODE_RD(Inst);
  CompInst.rs2     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm     = 0x00;

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCIInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = DECODE_RD(Inst);
  CompInst.rs1     = DECODE_RD(Inst);
  CompInst.imm     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm    |= ((Inst & 0b1000000000000)>>7);

  if((CompInst.opcode == 0b10) &&
     (CompInst.funct3 == 0b001)){
    // c.fldsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1100000) >> 2);        // [4:3]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    CompInst.imm |= ((Inst & 0b11100) << 4);          // [8:6]
  }else if( (CompInst.opcode == 0b10) &&
            (CompInst.funct3 == 0b010) ){
    // c.lwsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1110000) >> 2);        // [4:2]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    CompInst.imm |= ((Inst & 1100) << 4);             // [7:6]
  }else if( (CompInst.opcode == 0b10) &&
            (CompInst.funct3 == 0b011) ){
    CompInst.imm = 0;
    if( feature->GetXlen() == 64 ){
      // c.ldsp
      CompInst.imm =  ((Inst & 0b1100000) >> 2);        // [4:3]
      CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
      CompInst.imm |= ((Inst & 0b11100) << 4);          // [8:6]
    }else{
      // c.flwsp
      CompInst.imm =  ((Inst & 0b1110000) >> 2);        // [4:2]
      CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
      CompInst.imm |= ((Inst & 1100) << 4);             // [7:6]
    }
  }else if( (CompInst.opcode == 0b01) &&
      (CompInst.funct3 == 0b011) &&
      (CompInst.rd == 2)){
    // c.addi16sp
    // swizzle: nzimm[4|6|8:7|5] nzimm[9]
    CompInst.imm = 0;
    CompInst.imm = ((Inst & 0b1000000) >> 2); // bit 4
    CompInst.imm |= ((Inst & 0b100) << 3);    // bit 5
    CompInst.imm |= ((Inst & 0b100000) << 1); // bit 6
    CompInst.imm |= ((Inst & 0b11000) << 4);  // bit 8:7
    CompInst.imm |= ((Inst & 0b1000000000000) >> 3);  // bit 9
    if( (CompInst.imm & 0b1000000000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111111111111000000000;
    }
  }else if( (CompInst.opcode == 0b01) &&
      (CompInst.funct3 == 0b011)  &&
      (CompInst.rd != 0) && (CompInst.rd != 2) ){
    // c.lui
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111100) << 10);       // [16:12]
    CompInst.imm |= ((Inst & 0b1000000000000) << 5);  // [17]
    if( (CompInst.imm & 0b100000000000000000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111000000000000000000;
    }
    CompInst.imm >>= 12;  //immd value will be re-aligned on execution
  }else if( (CompInst.opcode == 0b01) &&
            (CompInst.funct3 == 0b010) &&
            (CompInst.rd != 0) ){
    // c.li
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111100) >> 2);        // [4:0]
    CompInst.imm |= ((Inst & 0b1000000000000) >> 7);  // [5]
    if( (CompInst.imm & 0b100000) > 0 ){
      // sign extend
      CompInst.imm |= 0b11111111111111111111111111000000;
    }
  }else if( (CompInst.imm & 0b100000) > 0 ){
    // sign extend
    CompInst.imm |= 0b11111111111111111111111111100000;
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCSSInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs2     = DECODE_LOWER_CRS2(Inst);
  CompInst.imm     = ((Inst & 0b1111110000000) >> 7);

  if( CompInst.funct3 == 0b101 ){
    // c.fsdsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1110000000000) >> 7);    // [5:3]
    CompInst.imm |= ((Inst & 0b1110000000) >> 1);       // [8:6]
  }else if( CompInst.funct3 == 0b110 ){
    // c.swsp
    CompInst.imm = 0;
    CompInst.imm =  ((Inst & 0b1111000000000) >> 7);    // [5:2]
    CompInst.imm |= ((Inst & 0b110000000) >> 1);        // [7:6]
  }else if( CompInst.funct3 == 0b111 ){
    CompInst.imm = 0;
    if( feature->GetXlen() == 64 ){
      // c.sdsp
      CompInst.imm =  ((Inst & 0b1110000000000) >> 7);    // [5:3]
      CompInst.imm |= ((Inst & 0b1110000000) >> 1);       // [8:6]
    }else{
      // c.fswsp
      CompInst.imm =  ((Inst & 0b1111000000000) >> 7);    // [5:2]
      CompInst.imm |= ((Inst & 0b110000000) >> 1);        // [7:6]
    }
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCIWInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = ((Inst & 0b11100) >> 2);
  CompInst.imm     = ((Inst & 0b1111111100000) >> 5);

  //swizzle: nzuimm[5:4|9:6|2|3]
  std::bitset<32> imm(CompInst.imm);
  std::bitset<32> tmp(0);
  tmp[0] = imm[1];
  tmp[1] = imm[0];
  tmp[2] = imm[6];
  tmp[3] = imm[7];
  tmp[4] = imm[2];
  tmp[5] = imm[3];
  tmp[6] = imm[4];
  tmp[7] = imm[5];

  CompInst.imm = tmp.to_ulong();

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCLInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rd      = ((Inst & 0b11100) >> 2);
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);

  if( CompInst.funct3 == 0b001 ){
    // c.fld
    CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b010 ){
    // c.lw
    CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
    CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b011 ){
    if( feature->GetXlen() == 64 ){
      // c.ld
      CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }else{
      // c.flw
      CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
      CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }
  }else if( CompInst.funct3 == 0b101 ){
    // c.fsd
    CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b110 ){
    // c.sw
    CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
    CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
    CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
  }else if( CompInst.funct3 == 0b111 ){
    if( feature->GetXlen() == 64 ){
      // c.sd
      CompInst.imm =  ((Inst & 0b1100000) << 1);        // [7:6]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }else{
      // c.fsw
      CompInst.imm =  ((Inst & 0b100000) << 1);         // [6]
      CompInst.imm |= ((Inst & 0b1000000) >> 4);        // [2]
      CompInst.imm |= ((Inst & 0b1110000000000) >> 7);  // [5:3]
    }
  }


  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCSInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs2     = ((Inst & 0b11100) >> 2);
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);

  if(CompInst.funct3 == 0b110){
    CompInst.imm     = ((Inst & 0b0100000) >> 1);         //offset[6]
    CompInst.imm    |= ((Inst & 0b1110000000000) >> 6);   //offset[5:3]
    CompInst.imm    |= ((Inst & 0b1000000) >> 6);          //offset[2]
  }else{
    CompInst.imm     = ((Inst & 0b1100000) >> 2);
    CompInst.imm    |= ((Inst & 0b1110000000000) >> 10);
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCAInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct2  = InstTable[Entry].funct2;
  CompInst.funct6  = InstTable[Entry].funct6;

  // registers
  CompInst.rs2     = ((Inst & 0b11100) >> 2);
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);
  CompInst.rd      = ((Inst & 0b1110000000) >> 7);

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCBInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);
  CompInst.offset  = ((Inst & 0b1111100) >> 2);
  CompInst.offset |= ((Inst & 0b1110000000000) >> 5);

  //swizzle: offset[8|4:3]  offset[7:6|2:1|5]
  std::bitset<16> tmp(0);
  std::bitset<16> o(CompInst.offset);
  tmp[0] = o[1];
  tmp[1] = o[2];
  tmp[2] = o[5];
  tmp[3] = o[6];
  tmp[4] = o[0];
  tmp[5] = o[3];
  tmp[6] = o[4];
  tmp[7] = o[7];

  CompInst.offset = (uint16_t)tmp.to_ulong();

  // handle c.beqz/c.bnez offset
  if( (CompInst.opcode = 0b01) && (CompInst.funct3 >= 0b110) ){
    CompInst.offset = 0;  // reset it
    CompInst.offset = ((Inst & 0b11000) >> 2);          // [2:1]
    CompInst.offset |= ((Inst & 0b110000000000) >> 7);  // [4:3]
    CompInst.offset |= ((Inst & 0b100) << 3);           // [5]
    CompInst.offset |= ((Inst & 0b1100000) << 1);       // [7:6]
    CompInst.offset |= ((Inst & 0b1000000000000) >> 4); // [8]

    if( (CompInst.offset & 0b100000000) > 0 ){
      // sign extend
      CompInst.offset |= 0b11111111100000000;
    }
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCJInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  uint16_t offset = ((Inst & 0b1111111111100) >> 2);

  //swizzle bits offset[11|4|9:8|10|6|7|3:1|5]
  std::bitset<16> offsetBits(offset);
  std::bitset<16> target;
  target.reset();
  target[0] = offsetBits[1];
  target[1] = offsetBits[2];
  target[2] = offsetBits[3];
  target[3] = offsetBits[9];
  target[4] = offsetBits[0];
  target[5] = offsetBits[5];
  target[6] = offsetBits[4];
  target[7] = offsetBits[7];
  target[8] = offsetBits[8];
  target[9] = offsetBits[6];
  target[10] = offsetBits[10];
  CompInst.jumpTarget = ((u_int16_t)target.to_ulong()) << 1;
  //CompInst.jumpTarget = ((u_int16_t)target.to_ulong());

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCompressed(uint32_t Inst){
  uint16_t TmpInst = (uint16_t)(Inst&0b1111111111111111);
  RevInst CInst;
  uint8_t opc     = 0;
  uint8_t funct2  = 0;
  uint8_t funct3  = 0;
  uint8_t funct4  = 0;
  uint8_t funct6  = 0;
  uint8_t l3      = 0;
  uint32_t Enc    = 0x00ul;
  uint64_t PC     = GetPC();


  // decode the opcode
  opc = (TmpInst & 0b11);
  l3  = ((TmpInst & 0b1110000000000000)>>13);
  if( opc == 0b00 ){
    // quadrant 0
    funct3 = l3;
  }else if( opc == 0b01){
    // quadrant 1
    if( l3 <= 0b011 ){
      // upper portion: misc
      funct3 = l3;
    }else if( (l3 > 0b011) && (l3 < 0b101) ){
      // middle portion: arithmetics
      funct2 = ((TmpInst & 0b110000000000) >> 10);
      if( funct2 == 0b11 ){
        funct6 = ((TmpInst & 0b1111110000000000) >> 10);
        funct2 = 0b00;
      }else{
        funct3 = l3;
      }
    }else{
      // lower power: jumps/branches
      funct3 = l3;
    }
  }else if( opc == 0b10){
    // quadrant 2
    if( l3 == 0b000 ){
      // slli{64}
      funct3 = l3;
    }else if( l3 < 0b100 ){
      // float/double/quad load
      funct3 = l3;
    }else if( l3 == 0b100 ){
      // jump, mv, break, add
      funct4 = ((TmpInst & 0b1111000000000000) >> 12);
    }else{
      // float/double/quad store
      funct3 = l3;
    }
  }

  Enc |= (uint32_t)(opc);
  Enc |= (uint32_t)(funct2 << 2);
  Enc |= (uint32_t)(funct3 << 4);
  Enc |= (uint32_t)(funct4 << 8);
  Enc |= (uint32_t)(funct6 << 12);

  std::map<uint32_t,unsigned>::iterator it = CEncToEntry.find(Enc);
  if( it == CEncToEntry.end() ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%d\n opc=%x; funct2=%x, funct3=%x, funct4=%x, funct6=%x\n",
                  PC,
                  Enc, opc, funct2, funct3, funct4, funct6 );
  }

  unsigned Entry = it->second;
  if( Entry > (InstTable.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 "\
                  Opcode = %x Funct2 = %x Funct3 = %x Funct4 = %x Funct6 = %x Enc = %x \n", \
                  PC, opc, funct2, funct3, funct4, funct6, Enc );

  }

  RegFile(HartToDecode)->Entry = Entry;

  RegFile(HartToDecode)->trigger = false;

  switch( InstTable[Entry].format ){
  case RVCTypeCR:
    return DecodeCRInst(TmpInst,Entry);
    break;
  case RVCTypeCI:
    return DecodeCIInst(TmpInst,Entry);
    break;
  case RVCTypeCSS:
    return DecodeCSSInst(TmpInst,Entry);
    break;
  case RVCTypeCIW:
    return DecodeCIWInst(TmpInst,Entry);
    break;
  case RVCTypeCL:
    return DecodeCLInst(TmpInst,Entry);
    break;
  case RVCTypeCS:
    return DecodeCSInst(TmpInst,Entry);
    break;
  case RVCTypeCA:
    return DecodeCAInst(TmpInst,Entry);
    break;
  case RVCTypeCB:
    return DecodeCBInst(TmpInst,Entry);
    break;
  case RVCTypeCJ:
    return DecodeCJInst(TmpInst,Entry);
    break;
  default:
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction format at PC=%" PRIx64 ".", PC );
    break;
  }
}

RevInst RevProc::DecodeRInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = InstTable[Entry].funct7;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  if( (InstTable[Entry].imm == FImm) && (InstTable[Entry].rs2Class == RegUNKNOWN)){
    DInst.imm  = DECODE_IMM12(Inst) & 0b011111; 
  }else{
    DInst.imm     = 0x0;
  }

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode the atomic RL/AQ fields
  if( DInst.opcode == 0b0101111 ){
    DInst.rl = DECODE_RL(Inst);
    DInst.aq = DECODE_AQ(Inst);
  }

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }

  DInst.compressed = false;

  return DInst;
}

RevInst RevProc::DecodeIInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }

  // imm
  DInst.imm     = DECODE_IMM12(Inst);

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }
  DInst.compressed = false;

  return DInst;
}

RevInst RevProc::DecodeSInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  DInst.imm     = (DECODE_RD(Inst) | (DECODE_FUNCT7(Inst)<<5));

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  // Decode any ancillary SP/DP float options
  if( IsFloat(Entry) ){
    DInst.rm = DECODE_FUNCT3(Inst);
  }

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeUInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = 0x0;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }

  // imm
  DInst.imm     = DECODE_IMM20(Inst);

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeBInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }

  // imm
  //DInst.imm     = twos_compl((DECODE_RD(Inst) | (DECODE_FUNCT7(Inst)<<5)),12);
  DInst.imm =   (uint32_t)((Inst << 4)&0b100000000000)|   // [11]
                (uint32_t)((Inst & 0b111100000000)>>7)|   // [4:1]
                (uint32_t)((Inst >> 20)&0b11111100000)|   // [10:5]
                (uint32_t)((Inst >> 19)&0b1000000000000);  // [12]

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeJInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = 0x0;
  DInst.funct7  = 0x0;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }

  // immA
  DInst.imm     = 0x00;
  DInst.imm     = ( (uint32_t)((Inst >> 20) & 0b11111111110) |            // imm[10:1]
                    (uint32_t)(Inst & 0b11111111000000000000) |           // imm[19:12]
                    (uint32_t)((Inst >> 9) & 0b100000000000) |            // imm[11]
                    (uint32_t)((Inst >> 11) & 0b100000000000000000000) ); // imm[20]

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevProc::DecodeR4Inst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile(HartToDecode)->cost  = InstTable[Entry].cost;

  // encodings
  DInst.opcode  = InstTable[Entry].opcode;
  DInst.funct3  = InstTable[Entry].funct3;
  DInst.funct2  = DECODE_FUNCT2(Inst);
  DInst.funct7  = InstTable[Entry].funct7;

  // registers
  DInst.rd      = 0x0;
  DInst.rs1     = 0x0;
  DInst.rs2     = 0x0;
  DInst.rs3     = 0x0;

  if( InstTable[Entry].rdClass != RegUNKNOWN ){
    DInst.rd  = DECODE_RD(Inst);
  }
  if( InstTable[Entry].rs1Class != RegUNKNOWN ){
    DInst.rs1  = DECODE_RS1(Inst);
  }
  if( InstTable[Entry].rs2Class != RegUNKNOWN ){
    DInst.rs2  = DECODE_RS2(Inst);
  }
  if( InstTable[Entry].rs3Class != RegUNKNOWN ){
    DInst.rs3  = DECODE_RS3(Inst);
  }

  // imm
  DInst.imm     = 0x0;

  // SP/DP Float
  DInst.fmt     = 0;
  DInst.rm      = 0;

  // Size
  DInst.instSize  = 4;

  DInst.compressed = false;
  return DInst;
}

bool RevProc::DebugReadReg(unsigned Idx, uint64_t *Value){
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    *Value = RegFile(HartToExec)->RV32[Idx];
    return true;
  }else{
    *Value = RegFile(HartToExec)->RV64[Idx];
    return true;
  }
}

bool RevProc::DebugWriteReg(unsigned Idx, uint64_t Value){
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    RegFile(HartToExec)->RV32[Idx] = (uint32_t)(Value&0xFFFFFFFF);
    return true;
  }else{
    RegFile(HartToExec)->RV64[Idx] = Value;
    return true;
  }
}

uint64_t RevProc::GetPC(){
  if( feature->GetXlen() == 32 ){
    return (uint64_t)(RegFile(HartToDecode)->RV32_PC);
  }else{
    return RegFile(HartToDecode)->RV64_PC;
  }
}

void RevProc::SetPC(uint64_t PC){
  if( feature->GetXlen() == 32 ){
    RegFile(HartToDecode)->RV32_PC = (uint32_t)(PC);
  }else{
    RegFile(HartToDecode)->RV64_PC = PC;
  }
}

bool RevProc::PrefetchInst(){
  // std::cout << "=========================" << std::endl;
  // std::cout << "PrefetchInst " << std::endl;
  // std::cout << "=========================" << std::endl;

  uint64_t PC   = 0x00ull;
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile(HartToDecode)->RV32_PC);
  }else{
    PC = RegFile(HartToDecode)->RV64_PC;
  }

  // std::cout << "- PC = 0x" << std::hex << PC << std::endl;


  // These are addresses that we can't decode
  // Return false back to the main program loop
  if( (PC == 0x00ull) ||
      (PC == _PAN_FWARE_JUMP_) ){
    return false;
  }

  // Check if instruction is in stream cache, if not - fetch it and stall
  return sfetch->IsAvail(PC);
}

RevInst RevProc::DecodeInst(){
  uint32_t Enc  = 0x00ul;
  uint32_t Inst = 0x00ul;
  uint64_t PC   = 0x00ull;
  bool Fetched  = false;

  // Stage 1: Retrieve the instruction
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile(HartToDecode)->RV32_PC);
  }else{
    PC = RegFile(HartToDecode)->RV64_PC;
  }

  if( !sfetch->InstFetch(PC, Fetched, Inst) ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve prefetched instruction at PC=0x%" PRIx64 "\n",
                  PC);
  }

#if 0
  if( !mem->ReadMem( PC, 4, (void *)(&Inst)) ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve instruction at PC=0x%" PRIx64 ".", PC );
  }
#endif

  output->verbose(CALL_INFO, 6, 0,
                  "Core %d ; Hart %d; PID %d; PC:InstPayload = 0x%" PRIx64 ":0x%" PRIx32 "\n",
                  id, HartToDecode, ActivePIDs.at(HartToDecode), PC, Inst);

  // Stage 1a: handle the crack fault injection
  if( CrackFault ){
    srand(time(NULL));
    uint64_t rval = rand() % (2^(fault_width));
    Inst |= rval;

    // clear the fault
    CrackFault = false;
  }

  // Stage 2: Retrieve the opcode
  const uint32_t Opcode = (uint32_t)(Inst&0b1111111);

  // If we find a compressed instruction, then take
  // the compressed decode path
  if( (Opcode&0b11) != 0b11 ){
    // this is a compressed instruction
    return DecodeCompressed(Inst);
  }

  // Stage 3: Determine if we have a funct3 field
  uint32_t Funct3 = 0x00ul;
  const uint32_t inst42 = ((Opcode&0b11100) >> 2);
  const uint32_t inst65 = ((Opcode&0b1100000) >> 5);

  if( (inst42 == 0b011) && (inst65 == 0b11) ){
    // JAL
    Funct3 = 0x00ul;
  }else if( (inst42 == 0b101) && (inst65 == 0b00) ){
    // AUIPC
    Funct3 = 0x00ul;
  }else if( (inst42 == 0b101) && (inst65 == 0b01) ){
    // LUI
    Funct3 = 0x00ul;
  }else{
    // Retrieve the field
    Funct3 = ((Inst&0b111000000000000) >> 12 );
  }

  // Stage 4: Determine if we have a funct7 field (R-Type and some specific I-Type)
  uint32_t Funct7 = 0x00ul;
  if( inst65 == 0b01 ) {
    if( (inst42 == 0b011) || (inst42 == 0b100) || (inst42 == 0b110) ){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
    }
  }else if((inst65== 0b10) && (inst42 == 0b100)){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
  }else if((inst65 == 0b00) && (inst42 == 0b110) && (Funct3 != 0)){
      // R-Type encodings
      Funct7 = ((Inst >> 25) & 0b1111111);
  }else if((inst65 == 0b00) && (inst42 == 0b100) && (Funct3 == 0b101)){
      // Special I-Type encoding for SRAI - also, Funct7 is only 6 bits in this case
      Funct7 = ((Inst >> 26) & 0b1111111);
  }

  // Stage 5: Determine if we have an imm12 field
  uint32_t Imm12 = 0x00ul;
  if( (inst42 == 0b100) && (inst65 == 0b11)  && (Funct3 == 0)){
    Imm12 = ((Inst >> 19) & 0b111111111111);
  }

  // Stage 6: Compress the encoding
  Enc |= Opcode;
  Enc |= (Funct3<<8);
  Enc |= (Funct7<<11);
  Enc |= (Imm12<<18);

  // Stage 7: Look up the value in the table
  std::map<uint32_t,unsigned>::iterator it;
  it = EncToEntry.find(Enc);
   if( it == EncToEntry.end() && ((Funct3 == 7) || (Funct3==1)) && (inst65 == 0b10)){
    //This is kind of a hack, but we may not have found the instruction becasue
    //  Funct3 is overloaded with rounding mode, so if this is a RV32F or RV64F
    //  set Funct3 to zero and check again
    Enc = 0;
    Enc |= Opcode;
    Enc |= (Funct7<<11);
    Enc |= (Imm12<<18);
    it = EncToEntry.find(Enc);
    if( it == EncToEntry.end() ){
      // failed to decode the instruction
      output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%d\n",
                  PC,
                  Enc );
    }

  }

  unsigned Entry = it->second;

  if( Entry > (InstTable.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 " \
                  Opcode = %x Funct3 = %x Funct7 = %x Imm12 = %x Enc = %x \n", \
                  PC, Opcode, Funct3, Funct7, Imm12, Enc );

  }

  RegFile(HartToDecode)->Entry = Entry;

  RegFile(HartToDecode)->trigger = false;

  // Stage 8: Do a full deocode using the target format
  switch( InstTable[Entry].format ){
  case RVTypeR:
    return DecodeRInst(Inst,Entry);
    break;
  case RVTypeI:
    return DecodeIInst(Inst,Entry);
    break;
  case RVTypeS:
    return DecodeSInst(Inst,Entry);
    break;
  case RVTypeU:
    return DecodeUInst(Inst,Entry);
    break;
  case RVTypeB:
    return DecodeBInst(Inst,Entry);
    break;
  case RVTypeJ:
    return DecodeJInst(Inst,Entry);
    break;
  case RVTypeR4:
    return DecodeR4Inst(Inst,Entry);
    break;
  default:
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction format at PC=%" PRIx64 ".", PC );
    break;
  }
}

void RevProc::ResetInst(RevInst *I){
  I->opcode     = 0;
  I->funct2     = 0;
  I->funct3     = 0;
  I->funct4     = 0;
  I->funct6     = 0;
  I->funct7     = 0;
  I->rd         = ~0;  // Set registers to value that is clearly invalid
  I->rs1        = ~0;
  I->rs2        = ~0;
  I->rs3        = ~0;
  I->imm        = 0;
  I->fmt        = 0;
  I->rm         = 0;
  I->aq         = 0;
  I->rl         = 0;
  I->offset     = 0;
  I->jumpTarget = 0;
  I->instSize   = 0;
  I->compressed = false;
}

void RevProc::HandleRegFault(unsigned width){
  // build the permissible set of registers available to fault
  unsigned LWidth = 0;
  std::vector<std::pair<std::string,void*>> RRegs;

  if( feature->GetXlen() == 32 ){
    if( width > feature->GetXlen() ){
      LWidth = feature->GetXlen();
    }else{
      LWidth = width;
    }

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile(HartToExec)->RV32[i])));
    }
  }else{
    // rv64
    LWidth = width;

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile(HartToExec)->RV64[i])));
    }
  }

  if( feature->IsModeEnabled(RV_F) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile(HartToExec)->SPF[i])));
    }
  }else if( feature->IsModeEnabled(RV_D) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&RegFile(HartToExec)->DPF[i])));
    }
  }

  // build the payload
  srand(time(NULL));
  uint64_t rval = rand() % (2^(LWidth));

  // select a register
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(0, RRegs.size()-1); // define the range
  unsigned RegIdx = distr(gen);

  std::string RegName = RRegs[RegIdx].first;
  if( feature->GetXlen() == 32 ){
    uint32_t *ptr = (uint32_t *)(RRegs[RegIdx].second);
    *ptr |= (uint32_t)(rval);
  }else{
    uint64_t *ptr = (uint64_t *)(RRegs[RegIdx].second);
    *ptr |= rval;
  }

  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:REG: Register fault of %d bits into register %s\n",
                  LWidth,RegName.c_str());
}

void RevProc::HandleCrackFault(unsigned width){
  CrackFault = true;
  fault_width = width;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:CRACK: Crack+Decode fault injected into next decode cycle\n");
}

void RevProc::HandleALUFault(unsigned width){
  ALUFault = true;
  fault_width = true;
  output->verbose(CALL_INFO, 5, 0,
                  "FAULT:ALU: ALU fault injected into next retire cycle\n");
}

bool RevProc::DependencyCheck(uint16_t threadID, RevInst* I){

      bool depFound = false;
      bool isFloat = IsFloat(I->entry);


      if(feature->IsRV32()){
        if(isFloat){
          depFound = (I->rs1 <= _REV_NUM_REGS_) ? RegFile(threadID)->SPF_Scoreboard[I->rs1] || depFound : depFound;
          depFound = (I->rs2 <= _REV_NUM_REGS_) ? RegFile(threadID)->SPF_Scoreboard[I->rs2] || depFound : depFound;
          depFound = (I->rs3 <= _REV_NUM_REGS_) ? RegFile(threadID)->SPF_Scoreboard[I->rs3] || depFound : depFound;
        }else{
          depFound = (I->rs1 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV32_Scoreboard[I->rs1] || depFound : depFound;
          depFound = (I->rs2 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV32_Scoreboard[I->rs2] || depFound : depFound;
          depFound = (I->rs3 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV32_Scoreboard[I->rs3] || depFound : depFound;
        }
      }else {
        if(isFloat){
          depFound = (I->rs1 <= _REV_NUM_REGS_) ? RegFile(threadID)->DPF_Scoreboard[I->rs1] || depFound : depFound;
          depFound = (I->rs2 <= _REV_NUM_REGS_) ? RegFile(threadID)->DPF_Scoreboard[I->rs2] || depFound : depFound;
          depFound = (I->rs3 <= _REV_NUM_REGS_) ? RegFile(threadID)->DPF_Scoreboard[I->rs3] || depFound : depFound;
        }else{
          depFound = (I->rs1 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV64_Scoreboard[I->rs1] || depFound : depFound;
          depFound = (I->rs2 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV64_Scoreboard[I->rs2] || depFound : depFound;
          depFound = (I->rs3 <= _REV_NUM_REGS_) ? RegFile(threadID)->RV64_Scoreboard[I->rs3] || depFound : depFound;
        }
      }
    return depFound;
}

void RevProc::DependencySet(uint16_t threadID, RevInst* Inst){
      if(Inst->rd > 0 && Inst->rd < _REV_NUM_REGS_){
        bool isFloat = IsFloat(Inst->entry);
        if(feature->IsRV32()){
          if(isFloat){
            RegFile(threadID)->SPF_Scoreboard[Inst->rd] = true;
          }else{
            RegFile(threadID)->RV32_Scoreboard[Inst->rd] = true;
          }
      }else{
          if(isFloat){
            RegFile(threadID)->DPF_Scoreboard[Inst->rd] = true;
          }else{
            RegFile(threadID)->RV64_Scoreboard[Inst->rd] = true;
          }
      }
    }
}

void RevProc::DependencyClear(uint16_t threadID, RevInst* Inst){
  if(Inst->rd < _REV_NUM_REGS_){
    bool isFloat = IsFloat(Inst->entry);
    if(feature->IsRV32()){
      if(isFloat){
        RegFile(threadID)->SPF_Scoreboard[Inst->rd] = false;
      }else{
        RegFile(threadID)->RV32_Scoreboard[Inst->rd] = false;
      }
    }else{
      if(isFloat){
        RegFile(threadID)->DPF_Scoreboard[Inst->rd] = false;
      }else{
        RegFile(threadID)->RV64_Scoreboard[Inst->rd] = false;
      }
    }
  }
}

uint32_t RevProc::HartToExecActivePID(){
  return ActivePIDs.at(HartToExec);
}

RevRegFile* RevProc::RegFile(){
  uint16_t HartID = GetHartID();
  auto it = ThreadTable.find(ActivePIDs.at(HartID));
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> Ctx = it->second;
    return Ctx->GetRegFile();
  }
  else {
    output->fatal(CALL_INFO, -1,
                  "Failed to find RegFile for PID = %d on Hart = %d \n", ActivePIDs.at(HartID), HartID);
  }
}

RevRegFile* RevProc::RegFile(uint16_t HartID){
  auto it = ThreadTable.find(ActivePIDs.at(HartID));
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> Ctx = it->second;
    return Ctx->GetRegFile();
  }
  else {
    output->fatal(CALL_INFO, -1,
                  "Failed to find RegFile for PID = %d on Hart = %d \n", ActivePIDs.at(HartID), HartID);
  }
}

uint16_t RevProc::GetHartID(){

  uint16_t nextID = HartToExec;
  if(HART_CTS[HartToExec]){
    nextID = HartToExec;
  }else{
    for(int tID = 0; tID < _REV_HART_COUNT_; tID++){
      nextID++;
      if(nextID >= _REV_HART_COUNT_){
        nextID = 0;
      }
      if(HART_CTS[nextID]){ break; };
    }
    output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; Hart switch from %d to %d \n",
                    id, HartToExec, nextID);
  }
  return nextID;
}

bool RevProc::ClockTick( SST::Cycle_t currentCycle ){
  bool rtn = false;
  Stats.totalCycles++;

  if( PendingCtxSwitch ){
    // std::cout << "=============================================" << std::endl;
    std::cout << "THREAD EVENT: PendingCtxSwitch TRUE" << std::endl;
    // std::cout << "=============================================" << std::endl;
    // std::cout << "Address of Old RegFile = 0x" << std::hex << ThreadTable.at(ActivePIDs.at(HartToExec))->GetRegFile() << std::endl;
    // std::cout << "Address of New RegFile = 0x" << std::hex << ThreadTable.at(NextPID)->GetRegFile() << std::endl;
    // std::cout << "=============================================" << std::endl;

    if( Pipeline.empty() ) {
      ResetInst(&Inst);
      if( !ChangeActivePID(NextPID) ){
        std::cout << "Failed to change active PID" << std::endl;
      } else {
        std::cout << "Successfully Updated Hart " << HartToExec << " to PID = " << RegFile()->PID << std::endl;
        RegFile()->RV64_SCAUSE = 0;
        RegFile(HartToDecode)->trigger = 0;
        RegFile(HartToDecode)->cost = 0;
        RegFile(HartToDecode)->RV64_SCAUSE = 0;
        ExecPC = GetPC();
        PendingCtxSwitch = false;
        /* DELETE ME */
        NextPID = 0;
      }
    }
  }

  #ifdef _REV_DEBUG_
  if((currentCycle % 100000000) == 0){
    std::cout << "Current Cycle: " << currentCycle <<  " PC: " << std::hex << ExecPC << std::dec << std::endl;
  }
  #endif

  // -- MAIN PROGRAM LOOP --
  //
  // If the clock is down to zero, then fetch the next instruction
  // else if the the instruction has not yet been triggered, execute it
  // else, wait until the counter is decremented to zero to retire the instruction
  //
  for (int tID = 0; tID < _REV_HART_COUNT_; tID++){
    HART_CTS[tID] = (RegFile(tID)->cost == 0);
  }

  if( HART_CTS.any() && (!Halted)) { // && (RegFile(threadID].cost == 0)){
    // fetch the next instruction
    //
    ResetInst(&Inst);

    //Determine the active thread
    HartToDecode = GetHartID();

    if( !PrefetchInst() ){
      Stalled = true;
      Stats.cyclesStalled++;
    }else{
      Stalled = false;
    }

    // If the next instruction is our special bounce address
    // DO NOT decode it.  It will decode to a bogus instruction.
    // We do not want to retire this instruction until we're ready
    if( (GetPC() != _PAN_FWARE_JUMP_) && (!Stalled) ){
      Inst = DecodeInst();
      Inst.entry = RegFile(HartToDecode)->Entry;
    }

    //Now that we have decoded the instruction, check for pipeline hazards
    if(Stalled || DependencyCheck(HartToDecode, &Inst)) {
      RegFile(HartToDecode)->cost = 0; // We failed dependency check, so set cost to 0 - this will
      Stats.cyclesIdle_Pipeline++;        // prevent the instruction from advancing to the next stage
      HART_CTE[HartToDecode] = false;
      HartToExec = _REV_INVALID_HART_ID_;
    }else {
      Stats.cyclesBusy++;
      HART_CTE[HartToDecode] = true;
      HartToExec = HartToDecode;
    };

    Inst.cost = RegFile(HartToDecode)->cost;
    Inst.entry = RegFile(HartToDecode)->Entry;
    rtn = true;
    ExecPC = GetPC();
  }

  /* This block is where an instruction actually gets executed */
  if ((HartToExec != _REV_INVALID_HART_ID_) && !Halted && 
      HART_CTE[HartToExec] && !RegFile(HartToExec)->trigger){

    // trigger the next instruction
    RegFile(HartToExec)->trigger = true;

    // pull the PC
    // output->verbose(CALL_INFO, 6, 0,
    //                 "Core %d ; Hart %d; Thread %d; Executing PC= 0x%" PRIx64 "\n",
    //                 id, HartToExec, ActivePID(), ExecPC);

    // attempt to execute the instruction as long as it is NOT
    // the firmware jump PC
    if( ExecPC != _PAN_FWARE_JUMP_ ){

      // Find the instruction extension
      std::map<unsigned,std::pair<unsigned,unsigned>>::iterator it;
      it = EntryToExt.find(RegFile(HartToExec)->Entry);
      if( it == EntryToExt.end() ){
        // failed to find the extension
        output->fatal(CALL_INFO, -1,
                    "Error: failed to find the instruction extension at PC=%" PRIx64 ".", ExecPC );
      }

      // found the instruction extension
      std::pair<unsigned,unsigned> EToE = it->second;
      RevExt *Ext = Extensions[EToE.first];
      std::shared_ptr<RevThreadCtx> ActiveCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

      // std::cout << "-----------------------------------------" << std::endl;
      // std::cout << "Ext->SetRegFile" << std::endl;
      // std::cout << "- Being passed a RegFile w/ Address = 0x" << ActiveCtx->GetRegFile()  << std::endl;
      // std::cout << "-----------------------------------------" << std::endl;
      
      Ext->SetRegFile(ActiveCtx->GetRegFile());

      // execute the instruction
      if( !Ext->Execute(EToE.second, Inst, HartToExec) ){
        output->fatal(CALL_INFO, -1,
                    "Error: failed to execute instruction at PC=%" PRIx64 ".", ExecPC );
      }

      /*
       * Exception Handling
       * - Currently this is only for ecall
      */
      if( (RegFile()->RV64_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ||
          (RegFile()->RV32_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ){ // Ecall found
        
          #ifdef _REV_DEBUG_
          std::cout << "Hart "<< HartToExec << " found ecall with code: " << code << std::endl;
          #endif

          /* Execute system call on this RevProc */
          ExecEcall(); 

          #ifdef _REV_DEBUG_
          std::cout << "Hart "<< HartToExec << " returned from ecall with code: " << rc << std::endl;
          #endif

        // } else {
        //   ExecEcall();
          #ifdef _REV_DEBUG_
          std::cout << "Hart "<< HartToExec << " found ecall with code: " << code << std::endl;
          #endif

          /* exception handled... zero the cause register */
          RegFile()->RV64_SCAUSE = 0;

          /* Execute system call on this RevProc */

          #ifdef _REV_DEBUG_
          std::cout << "Hart "<< HartToExec << " returned from ecall with code: " << rc << std::endl;
          #endif
        // }
      }

      if( !PendingCtxSwitch ){
      Pipeline.push(std::make_pair(HartToExec, Inst));
      }

      bool isFloat = false;
      if( (Ext->GetName() == "RV32F") ||
          (Ext->GetName() == "RV32D") ||
          (Ext->GetName() == "RV64F") ||
          (Ext->GetName() == "RV64D") ){
        Stats.floatsExec++;
        isFloat = true;
      }

      DependencySet(HartToExec, &Inst);

      // -------------------------------------------
      // inject the ALU fault
      // -------------------------------------------
      if( ALUFault ){
        // inject ALU fault
        RevExt *Ext = Extensions[EToE.first];
        if( (Ext->GetName() == "RV32F") ||
            (Ext->GetName() == "RV32D") ){
          // write an rv32 float rd
          uint32_t rval = rand() % (2^(fault_width));
          uint32_t tmp = (uint32_t)(RegFile(HartToExec)->SPF[Inst.rd]);
          tmp |= rval;
          RegFile(HartToExec)->SPF[Inst.rd] = (float)(tmp);
        }else if( (Ext->GetName() == "RV64F") ||
                  (Ext->GetName() == "RV64D") ){
          // write an rv64 float rd
          uint64_t rval = rand() % (2^(fault_width));
          uint64_t tmp = (uint64_t)(RegFile(HartToExec)->DPF[Inst.rd]);
          tmp |= rval;
          RegFile(HartToExec)->DPF[Inst.rd] = (double)(tmp);
        }else if( feature->GetXlen() == 32 ){
          // write an rv32 gpr rd
          uint32_t rval = rand() % (2^(fault_width));
          RegFile(HartToExec)->RV32[Inst.rd] |= rval;
        }else{
          // write an rv64 gpr rd
          uint64_t rval = rand() % (2^(fault_width));
          RegFile(HartToExec)->RV64[Inst.rd] |= rval;
        }

        // clear the fault
        ALUFault = false;
      }
      // -------------------------------------------
    }

    // if this is a singlestep, clear the singlestep and halt
    if( SingleStep ){
      SingleStep = false;
      Halted = true;
    }

  if( currentCycle >= 14 ){
    for( auto Ctx : ThreadTable){
      std::cout << "PID: " << Ctx.first << "|" << Ctx.second->GetRegFile()->PID << std::endl;
      std::cout << "ADDRESS OF REGFILE = " << Ctx.second->GetRegFile() << std::endl;
    }
  }
    rtn = true;
  }else{
    // wait until the counter has been decremented
    // note that this will continue to occur until the counter is drained
    // and the HART is halted
    output->verbose(CALL_INFO, 9, 0,
                    "Core %d ; No available thread to exec PC= 0x%" PRIx64 "\n",
                    id, ExecPC);
    rtn = true;
    Stats.cyclesIdle_Total++;
    if(HART_CTE.any()){
      Stats.cyclesIdle_MemoryFetch++;
    }
  }

  if(!Pipeline.empty() && Pipeline.front().second.cost > 0){
    Pipeline.front().second.cost--;
    if(Pipeline.front().second.cost == 0){
      uint16_t tID = Pipeline.front().first;
      output->verbose(CALL_INFO, 6, 0,
                    "Core %d ; HartID %d; PID %d, Retiring PC= 0x%" PRIx64 "\n",
                    id, tID, HartToExecActivePID(), ExecPC);
      Retired++;
      RevInst retiredInst = Pipeline.front().second;
      DependencyClear(tID, &retiredInst);
      Pipeline.pop();
      RegFile(tID)->cost = 0;
    }
  }

  /*for(int tID = 0; tID < _REV_HART_COUNT_; tID ++){
    //A thread that has successfully decoded an instruction AND has no dependencies will have
      // a cost > 0 as set by the decode stage
      if(RegFile(tID].cost > 0){   
        RegFile(tID].cost = RegFile(tID].cost - 1;
        if( RegFile(tID].cost == 0 ){
            output->verbose(CALL_INFO, 6, 0,
                      "Core %d ; ThreadID %d; Retiring PC= 0x%" PRIx64 "\n",
                      id, tID, ExecPC);
            Retired++;
            RegFile(tID].trigger = false;
        }
      }
  }*/

  // Check for completion states and new tasks
  if( (GetPC() == _PAN_FWARE_JUMP_) || (GetPC() == 0x00ull) ){
    /*
     * We need to check if the thread whose PC = 0x00ull is 
     * the only thread still alive
     *
     * We do this by checking its ParentPID = 0. 
     * If it is, we know we are actually done
     * Else: Transition control back to ParentPID 
    */


    // look for more work on the execution queue
      // if no work is found, don't update the PC
      // just wait and spin
      bool done = true;
      if( GetPC() == _PAN_FWARE_JUMP_ ){
        if( PExec != nullptr){
          uint64_t Addr = 0x00ull;
          unsigned Idx = 0;
          PanExec::PanStatus Status = PExec->GetNextEntry(&Addr,&Idx);
          switch( Status ){
          case PanExec::QExec:
            output->verbose(CALL_INFO, 5, 0,
                        "Core %d ; PAN Exec Jumping to PC= 0x%" PRIx64 "\n",
                        id, Addr);
            SetPC(Addr);
            done = false;
            break;
          case PanExec::QNull:
            // no work to do; spin on the firmware jump PC
            output->verbose(CALL_INFO, 6, 0,
                        "Core %d ; No PAN work to do; Jumping to PC= 0x%" PRIx64 "\n",
                        id, ExecPC);
            done = false;
            SetPC(_PAN_FWARE_JUMP_);
            break;
          case PanExec::QValid:
          case PanExec::QError:
            done = true;
          default:
            break;
          }
        }
      }else if( GetPC() == 0x00ull ) {
        // PAN execution contexts not enabled, this is our last PC

      /* 
       * The following block is used for 
       */
      uint32_t ParentPID = ThreadTable.at(ActivePIDs.at(HartToExec))->GetParentPID();
      if(ParentPID != 0 ){
        done = false;
        std::cout << "============================================ " << std::endl;
        std::cout << "THREAD EVENT:" << std::endl;
        std::cout << " - PID " << ActivePIDs.at(HartToExec) << " COMPLETED EXECUTION" << std::endl;
        std::cout << " - NewPID: " << ActivePIDs.at(HartToExec) << std::endl;
        std::cout << "============================================ "<< std::endl;
        CtxSwitchAlert(ParentPID);
        SwapToParent = true;
        ThreadTable.at(ActivePIDs.at(HartToExec))->SetState(ThreadState::Dead);
      } else {
        done = true;
      }
      }

      // determine if we have any outstanding memory requests
      if( mem->outstandingRqsts() ){
        done = false;
      }
      if( done ){
        // we are really done, return
        output->verbose(CALL_INFO,2,0,"Program execution complete\n");
        Stats.percentEff = float(Stats.cyclesBusy)/Stats.totalCycles;
        output->verbose(CALL_INFO,2,0,
                        "Program Stats: Total Cycles: %" PRIu64 " Busy Cycles: %" PRIu64 " Idle Cycles: %" PRIu64 " Eff: %f\n",
                        Stats.totalCycles, Stats.cyclesBusy,
                        Stats.cyclesIdle_Total, Stats.percentEff);
        output->verbose(CALL_INFO,3,0,"\t Bytes Read: %d Bytes Written: %d Floats Read: %d Doubles Read %d  Floats Exec: %" PRIu64 " Inst Retired: %" PRIu64 "\n", \
                                        mem->memStats.bytesRead, \
                                        mem->memStats.bytesWritten, \
                                        mem->memStats.floatsRead, \
                                        mem->memStats.doublesRead, \
                                        Stats.floatsExec,
                                        Retired);
        return false;
      }
    // }
  }

  return rtn;
}

bool RevProc::InitThreadTable(){
  /*
   * We need to create the first Ctx for each HART which will have the following attributes: 
   * - ParentPID = 0 : (Only the first thread on every RevProc has ParentPID = 0)
   * - MemStartAddr : Top of stack
   * - MemStartSize :
  */

  for( unsigned HartID=0; HartID<_REV_HART_COUNT_; HartID++){
    uint32_t ParentPID = 0;
    uint32_t FirstActivePID = mem->GetNewThreadPID();
    uint64_t StartingMemAddr = mem->GetStackTop(); // TODO: Verify this is okay
    uint64_t StartingMemSize = mem->DefaultThreadMemSize;

    // RevRegFile InitRegFile;
    // std::cout << "ADDRESS OF THE INIT REG FILE = 0x"
    //           << std::hex << (uint64_t)(&InitRegFile) << std::dec << std::endl;

    std::shared_ptr<RevThreadCtx> DefaultCtx = std::make_shared<RevThreadCtx>(
        FirstActivePID,
        ParentPID,
        StartingMemAddr,
        StartingMemSize);

    DefaultCtx->GetRegFile()->PID = FirstActivePID;
    ActivePIDs.emplace_back(FirstActivePID);
    /* Add to ThreadTable */
    ThreadTable.emplace(FirstActivePID, DefaultCtx);
    // RevRegFile tmp = 
    // std::cout << "================== InitThreadTable ==================" << std::endl;
    // std::cout << "FirstActivePID = " << FirstActivePID << std::endl;
    // std::cout << "Address of DefaultCtx.RegFile = 0x" << std::hex << DefaultCtx->GetRegFile() << std::dec << std::endl;
    // std::cout << "Address of Ctx.RegFile from ThreadTable = 0x"
    //           << std::shared_ptr<RevThreadCtx>(ThreadTable.at(FirstActivePID))->GetRegFile() << std::endl;
    // std::cout << "=====================================================" << std::endl;
  }

  return true;
}


/*
 * This function changes the active pid of HartToExec
 *
 * Returns:
 * - True if successfully changed
 * - False if not (ie. PID doesn't exist)
 *
 *
 * NOTES:
 * - This function automatically sets the new Ctx state to Running
 * - This function automatically sets old Ctx state to Waiting
 */
bool RevProc::ChangeActivePID(uint32_t NewPID){
  auto it = ThreadTable.find(NewPID);
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> NewCtx = it->second;
    // std::cout << "=============================================" << std::endl;
    // std::cout << "ChangeActivePID" << std::endl;
    // std::cout << "=============================================" << std::endl;
    // std::cout << "Address of new RegFile = 0x" << std::hex << NewCtx->GetRegFile() << std::endl;
    // std::cout << "=============================================" << std::endl;
    if( SwapToParent ){
      std::cout << "Removing ThreadCtx w/ PID = " << ActivePIDs.at(HartToExec) << " from the ThreadTable" << std::endl;
      ThreadTable.erase(ActivePIDs.at(HartToExec));  
    }
    ActivePIDs.at(HartToExec) = NewPID;
    ActivePIDs.at(HartToDecode) = NewPID;
    return true;
  }else{
    /* TODO: Maybe don't output fatal? */
    output->fatal(CALL_INFO, -1, "Failed to load ctx w/ PID=%d into Hart=%d because PID does not exist in ThreadTable", NewPID, HartToExec); 
    return false;
  }
}

/*
 * This function changes the active pid of HartID
 *
 * Returns:
 * - True if successfully changed
 * - False if not (ie. PID doesn't exist)
 *
 * NOTES:
 * - This function automatically sets the new Ctx state to Running
 * - This function automatically sets old Ctx state to Waiting
 */
bool RevProc::ChangeActivePID(uint32_t PID, uint16_t HartID){
  auto NewActiveCtx = ThreadTable.find(PID);
  if( NewActiveCtx != ThreadTable.end() ){
    if( ActivePIDs.size() >= HartID ){
      ActivePIDs.at(HartToExec) = PID;
      return true;
    } else {
    /* TODO: Maybe don't output fatal? */
      output->fatal(CALL_INFO, -1, "Failed to load ctx w/ PID=%d into Hart=%d because Hart does not exist", PID, HartToExec); 
      return false;
    }
  }else{
    /* TODO: Maybe don't output fatal? */
    output->fatal(CALL_INFO, -1, "Failed to load ctx w/ PID=%d into Hart=%d because PID does not exist in ThreadTable", PID, HartToExec); 
    return false;
  }
}

/* GetPIDS()
 * - Returns vector of all PIDs in the ThreadTable 
 */
std::vector<uint32_t> RevProc::GetPIDs(){
  std::vector<uint32_t> PIDs;
  for( const auto& Thread : ThreadTable ){
    PIDs.push_back(Thread.first);
  }
  return PIDs;
}

/* 
 * There are a few assumptions made by this function
 * - The Active Thread is the one creating the child 
 * - The child duplicates the parents RegFile
 * - Automatically adds ChildCtx to the current Procs ThreadTable  
 * - The new Child will start with ThreadState::Ready
 * - FIXME: - Default ChildMemSize = _DEFAULT_THREAD_MEM_SIZE (Maybe make it equal to ParentMemSize?)
 * - FIXME: - Starting Mem Address of the child = ParentsAddr + ParentsSize 
*/

uint32_t RevProc::CreateChildCtx() {
  // NOTE: Calling GetActiveCtx with no arguments returns the current HartToExec's Ctx
  std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

  RevRegFile* ParentRegfile = ParentCtx->GetRegFile();
  std::shared_ptr<RevRegFile> ChildRegFile; 

  uint32_t ChildPID = mem->GetNewThreadPID();
  std::cout << "New ChildPID = " << ChildPID << std::endl;

  // Create ChildCtx as a copy of ParentCtx
  auto ChildCtx = std::make_shared<RevThreadCtx>(ChildPID,
                                       ActivePIDs.at(HartToExec),
                                       ParentCtx->GetMemStartAddr(),
                                       ParentCtx->GetMemSize());

  ChildCtx->DuplicateRegFile(*ThreadTable.at(ActivePIDs.at(HartToExec))->GetRegFile());

  std::cout << "Func: CreateChildCtx (RevProc @ 2264) --- CHILD PID = " << ChildPID; 
  ThreadTable.emplace(ChildPID, ChildCtx);
  ThreadTable.at(ChildPID)->GetRegFile()->PID = ChildPID;
  ThreadTable.at(ChildPID)->GetRegFile()->RV64_SCAUSE = 0;
  ThreadTable.at(ChildPID)->GetRegFile()->RV32_SCAUSE = 0;
  return ChildPID;
}


/* ======================================================= */
/* System Call (ecall) Implementations Below
/* ======================================================= */
void RevProc::InitEcallTable(){
  Ecalls = {
    {17,  &RevProc::ECALL_getcwd},          // Not implemented
    {23,  &RevProc::ECALL_dup},             // Not implemented
    {24,  &RevProc::ECALL_dup3},            // Not implemented
    {34,  &RevProc::ECALL_mkdirat},         // Not implemented
    {49,  &RevProc::ECALL_chdir},          
    {54,  &RevProc::ECALL_fchownat},        // Not implemented
    {55,  &RevProc::ECALL_fchown},          // Not implemented
    {57,  &RevProc::ECALL_close},           // Not implemented
    {63,  &RevProc::ECALL_read},            // Not implemented
    {64,  &RevProc::ECALL_write},          
    {65,  &RevProc::ECALL_openat},          // Not implemented
    {77,  &RevProc::ECALL_tee},             // Not implemented
    {81,  &RevProc::ECALL_sync},            // Not implemented
    {82,  &RevProc::ECALL_fsync},           // Not implemented
    {83,  &RevProc::ECALL_fdatasync},       // Not implemented
    {93,  &RevProc::ECALL_exit},           
    {94,  &RevProc::ECALL_exit_group},      // Not implemented
    {95,  &RevProc::ECALL_waitid},          // Not implemented
    {99,  &RevProc::ECALL_set_robust_list}, // Not implemented
    {100, &RevProc::ECALL_get_robust_list}, // Not implementedt
    {101, &RevProc::ECALL_nanosleep},       // Not implemented
    {107, &RevProc::ECALL_timer_create},    // Not implemented
    {110, &RevProc::ECALL_timer_delete},    // Not implemented
    {135, &RevProc::ECALL_rt_sigprocmask},  // Not implemented
    {169, &RevProc::ECALL_gettimeofday},    // Not implemented
    {170, &RevProc::ECALL_settimeofday},    // Not implemented
    {172, &RevProc::ECALL_getpid},         
    {173, &RevProc::ECALL_getppid},        
    {178, &RevProc::ECALL_gettid},          // Not implemented
    {220, &RevProc::ECALL_clone},           // Fork functionality works but not clone3
    {222, &RevProc::ECALL_mmap},            // Not implemented
    {403, &RevProc::ECALL_clock_gettime},   // Not implemented
    {404, &RevProc::ECALL_clock_settime},   // Not implemented
    {408, &RevProc::ECALL_timer_gettime},   // Not implemented
    {409, &RevProc::ECALL_timer_settime},   // Not implemented
    };
}

void RevProc::ECALL_clone(){

  std::cout << "=======================================" << std::endl;
  std::cout << "CLONE: " << std::endl;
  
  uint64_t CloneArgsAddr = RegFile()->RV64[10];
  size_t SizeOfCloneArgs = RegFile()->RV64[11];
  
  std::cout << "Address of clone_args = 0x" << std::hex << (uint64_t)CloneArgsAddr << std::endl;
  std::cout << "Size of clone_args = 0x" << std::hex << SizeOfCloneArgs << std::endl;

  struct clone_args;

  uint64_t flags;

  mem->ReadMem(CloneArgsAddr, sizeof(uint64_t), &flags);

  std::cout << "- Flags = " << flags << std::endl;

  

  /* Get the parent ctx (Current active, executing PID) */
  std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));
  RevRegFile* ParentRegFile = ParentCtx->GetRegFile();

  /* Create the child ctx */
  uint32_t ChildPID = CreateChildCtx();
  std::shared_ptr<RevThreadCtx> ChildCtx = ThreadTable.at(ChildPID);


  /* Create a copy of Parents Memory Space */
  // const char* ParentMem[ParentCtx->GetMemSize()];
  // mem->ReadMem(ParentCtx->GetMemStartAddr(), ParentCtx->GetMemSize(), ParentMem);
  // mem->WriteMem(ChildCtx->GetMemStartAddr(), ParentCtx->GetMemSize(), ParentMem);

  /* 
   * ===========================================================================================
   * Register File
   * ===========================================================================================
   * We need to duplicate the parent's RegFile to to the Childs
   * - NOTE: when we return from this function, the return value will 
   *         be automatically stored in the Proc.RegFile[threadToExec]'s a0 
   *         register. In a traditional fork code this looks like:
   *         
   *         pid_t pid = fork()
   *         if pid < 0: // Error
   *         else if pid = 0: // New Child Process
   *         else: // Parent Process
   *        
   *         In this case, the value of pid is the value thats returned to a0
   *         It follows that 
   *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
   *         - The Parent's a0 register MUST have its PID in it 
   * ===========================================================================================
   */
    ChildCtx->GetRegFile()->RV64[10] = 0;
    ChildCtx->GetRegFile()->RV64_SCAUSE = 0;
    // RegFile = ChildCtx.RegFile;
    // Proc.ThreadTable.at(ParentPID).GetRegFile()->RV64[10] = ParentPID;

    /*
      * Alert the Proc there needs to be a Ctx switch
      * Pass the PID that will be switched to once the 
      * current pipeline is executed until completion
    */
  // }
  CtxSwitchAlert(ChildPID);

  /* Parent's return value is its own PID */
  ParentCtx->GetRegFile()->RV64[10] = ChildPID;
  ChildCtx->GetRegFile()->RV64[10] = 0; 
  /* Child's return value is 0 */
  std::cout << "=======================================" << std::endl;
  return;
}

/*  */
void RevProc::ECALL_chdir(){
  RevRegFile& regFile = *RegFile();

  std::cout << "ECALL_chdir called" << std::endl;
  
  std::string path = "";
  unsigned i=0;
  
  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    mem->ReadMem(regFile.RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = chdir(path.data());
  RegFile()->RV64[10] = rc;
}

void RevProc::ECALL_mkdir(){
  RevRegFile& regFile = *RegFile();

  std::cout << "ECALL_mkdir called" << std::endl;

  std::string path = "";
  unsigned i=0;
  
  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    mem->ReadMem(regFile.RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = chdir(path.data());
}

void RevProc::ECALL_exit(){
  std::cout << "ECALL_exit called" << std::endl;
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecActiveCtx();

  /* If the current ctx has ParentPID = 0, it has no parent and we should terminate the sim */
  if( CurrCtx->GetParentPID() == 0 ){
    const uint64_t status = RegFile()->RV64[10];
    const std::string ExitString = "ECALL: Encountered exit code with status = " + std::to_string(status) + "\n";
    output->fatal(CALL_INFO, -1, "%s", ExitString.c_str());
  } else {
    /* Parent exists & Child is exiting... switch back to parent */ 
    CtxSwitchAlert(CurrCtx->GetParentPID());
    return;
  }
  return;
}

void RevProc::ECALL_getcwd(){
  std::cout << "ECALL_getcwd called" << std::endl;
}

/* TODO: Implement error handling */
void RevProc::ECALL_getpid(){
  std::cout << "ECALL_getpid called" << std::endl;
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  CurrentCtx->GetRegFile()->RV64[10] = ActivePIDs.at(HartToExec);
  return;
}

/* TODO: Implement error handling */
void RevProc::ECALL_getppid(){
  std::cout << "ECALL_getppid called" << std::endl;
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  uint32_t ParentPID = CurrentCtx->GetParentPID();
  RegFile()->RV64[10] = ParentPID;
  return;
}


void RevProc::ECALL_write(){
  std::cout << "ECALL_write called" << std::endl;
  
  int fildes = RegFile()->RV64[10];

  std::size_t nbytes = RegFile()->RV64[12];

  char buf[nbytes];
  char bufchar;
  uint64_t MemOffset = ThreadTable.at(ActivePIDs.at(HartToExec))->GetMemStartAddr();
  for (unsigned i=0; i<nbytes; i++){
    mem->ReadMem(RegFile()->RV64[11] + sizeof(char)*i, sizeof(char), &bufchar);
    std::cout << "WRITE: Char Found: " << bufchar << std::endl; 
  }
  mem->ReadMem(RegFile()->RV64[11], sizeof(buf), &buf);

  const int rc = write(fildes, buf, nbytes);
  RegFile()->RV64[10] = rc;
}


void RevProc::ECALL_timer_settime(){
  std::cout << "ECALL: timer_settime called" << std::endl;
  return;
}

void RevProc::ECALL_timer_gettime(){
  std::cout << "ECALL: timer_gettime called" << std::endl;
  return;
}

void RevProc::ECALL_clock_settime(){
  std::cout << "ECALL: clock_settime called" << std::endl;
  return;
}

void RevProc::ECALL_clock_gettime(){
  std::cout << "ECALL: clock_gettime called" << std::endl;
  return;
}

void RevProc::ECALL_mmap(){
  std::cout << "ECALL: mmap called" << std::endl;
  return;
}

void RevProc::ECALL_gettid(){
  std::cout << "ECALL: gettid called" << std::endl;
  return;
}

void RevProc::ECALL_settimeofday(){
  std::cout << "ECALL: settimeofday called" << std::endl;
  return;
}

void RevProc::ECALL_gettimeofday(){
  std::cout << "ECALL: gettimeofday called" << std::endl;
  return;
}

void RevProc::ECALL_rt_sigprocmask(){
  std::cout << "ECALL: rt_sigprocmask called" << std::endl;
  return;
}

void RevProc::ECALL_timer_delete(){
  std::cout << "ECALL: timer_delete called" << std::endl;
  return;
}

void RevProc::ECALL_timer_create(){
  std::cout << "ECALL: timer_create called" << std::endl;
  return;
}

void RevProc::ECALL_nanosleep(){
  std::cout << "ECALL: nanosleep called" << std::endl;
  return;
}

void RevProc::ECALL_get_robust_list(){
  std::cout << "ECALL: get_robust_list called" << std::endl;
  return;
}

void RevProc::ECALL_set_robust_list(){
  std::cout << "ECALL: set_robust_list called" << std::endl;
  return;
}

void RevProc::ECALL_waitid(){
  std::cout << "ECALL: waitid called" << std::endl;
  return;
}

void RevProc::ECALL_exit_group(){
  std::cout << "ECALL: exit_group called" << std::endl;
  return;
}

void RevProc::ECALL_fdatasync(){
  std::cout << "ECALL: fdatasync called" << std::endl;
  return;
}

void RevProc::ECALL_fsync(){
  std::cout << "ECALL: fsync called" << std::endl;
  return;
}

void RevProc::ECALL_sync(){
  std::cout << "ECALL: sync called" << std::endl;
  return;
}

void RevProc::ECALL_tee(){
  std::cout << "ECALL: tee called" << std::endl;
  return;
}

void RevProc::ECALL_openat(){
  std::cout << "ECALL: openat called" << std::endl;
  return;
}

void RevProc::ECALL_read(){
  std::cout << "ECALL: read called" << std::endl;
  return;
}

void RevProc::ECALL_close(){
  std::cout << "ECALL: close called" << std::endl;
  return;
}

void RevProc::ECALL_fchown(){
  std::cout << "ECALL: fchown called" << std::endl;
  return;
}

void RevProc::ECALL_fchownat(){
  std::cout << "ECALL: fchownat called" << std::endl;
  return;
}

void RevProc::ECALL_mkdirat(){
  std::cout << "ECALL: mkdirat called" << std::endl;
  return;
}

void RevProc::ECALL_dup3(){
  std::cout << "ECALL: dup3 called" << std::endl;
  return;
}

void RevProc::ECALL_dup(){
  std::cout << "ECALL: dup called" << std::endl;
  return;
}


void RevProc::ExecEcall(){
  // a7 register = ecall code
  uint64_t EcallCode;
  if( feature->IsRV32() )
    EcallCode = (uint64_t)RegFile()->RV32[17];
  else if( feature->IsRV64() ) 
    EcallCode = RegFile()->RV64[17];
  else {
    return;
  }
  
  auto it = Ecalls.find(EcallCode);
  if( it != Ecalls.end() ){
    /* call the function */
    (it->second)(this);
    /* Trap handled... 0 cause registers */
    RegFile()->RV64_SCAUSE = 0;
    RegFile()->RV32_SCAUSE = 0;
  } else {
    output->fatal(CALL_INFO, -1, "Ecall Code = %d not found", EcallCode);
  }
}

// EOF
