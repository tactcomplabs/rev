//
// _RevProc_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevProc.h"
#include <bitset>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <fstream>
#include <sys/xattr.h>

RevProc::RevProc( unsigned Id,
                  RevOpts *Opts,
                  RevMem *Mem,
                  RevLoader *Loader,
                  RevCoProc* CoProc,
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
                  "Error: failed to retrieve the machine model for core=%u\n", id);

  unsigned MinCost = 0;
  unsigned MaxCost = 0;

  Opts->GetMemCost(Id,MinCost,MaxCost);

  if(CoProc){
    coProc = CoProc;
  }else{
    coProc = NULL;
  }

  feature = new RevFeature(Machine,output,MinCost,MaxCost,Id);
  if( !feature )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevFeature object for core=%u\n", id);

  unsigned Depth = 0;
  Opts->GetPrefetchDepth(Id, Depth);
  if( Depth == 0 ){
    Depth = 16;
  }

  sfetch = new RevPrefetcher(Mem,feature,Depth);
  if( !sfetch )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to create the RevPrefetcher object for core=%u\n", id);

  // Initialize ThreadTable (NOTE: Default PID = 1024 + ProcID)
  if( !InitThreadTable() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to initialize the ThreadTable for core=%u\n", id );

  // load the instruction tables
  if( !LoadInstructionTable() )
    output->fatal(CALL_INFO, -1,
                  "Error : failed to load instruction table for core=%u\n", id );

  // Initialize EcallTable
  InitEcallTable();
  if( Ecalls.size() <= 0 )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to initialize the Ecall Table for core=%u\n", id );

  // reset the core
  if( !Reset() )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to reset the core resources for core=%u\n", id );

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
  delete sfetch;
  delete feature;
}

RevProc::RevProcStats RevProc::GetStats(){
  Stats.memStats.bytesRead      = mem->memStats.bytesRead;
  Stats.memStats.bytesWritten   = mem->memStats.bytesWritten;
  Stats.memStats.doublesRead    = mem->memStats.doublesRead;
  Stats.memStats.doublesWritten = mem->memStats.doublesWritten;
  Stats.memStats.floatsRead     = mem->memStats.floatsRead;
  Stats.memStats.floatsWritten  = mem->memStats.floatsWritten;
  Stats.memStats.TLBMisses      = mem->memStats.TLBMisses;
  Stats.memStats.TLBHits        = mem->memStats.TLBHits;
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
                  "Core %u ; Enabling extension=%s\n",
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
                    "Core %u ; Enabling compressed extension=%s\n",
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
                      "Core %u ; Enabling optional compressed extension=%s\n",
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
                    "Core %u ; Seeding instruction table for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());


  // I-Extension
  if( feature->IsModeEnabled(RV_I) ){
    if( feature->GetXlen() == 64 ){
      // load RV32I & RV64; no optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile,mem,output)),false);
      EnableExt(static_cast<RevExt *>(new RV64I(feature,RegFile,mem,output)),false);
    }else{
      // load RV32I w/ optional compressed
      EnableExt(static_cast<RevExt *>(new RV32I(feature,RegFile,mem,output)),true);
    }
  }

  // M-Extension
  if( feature->IsModeEnabled(RV_M) ){
    EnableExt(static_cast<RevExt *>(new RV32M(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64M(feature,RegFile,mem,output)),false);
    }
  }

  // A-Extension
  if( feature->IsModeEnabled(RV_A) ){
    EnableExt(static_cast<RevExt *>(new RV32A(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64A(feature,RegFile,mem,output)),false);
    }
  }

  // F-Extension
  if( feature->IsModeEnabled(RV_F) ){
    if( (!feature->IsModeEnabled(RV_D)) && (feature->GetXlen() == 32) ){
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile,mem,output)),true);
    }else{
      EnableExt(static_cast<RevExt *>(new RV32F(feature,RegFile,mem,output)),false);
      EnableExt(static_cast<RevExt *>(new RV64F(feature,RegFile,mem,output)),false);

    }
#if 0
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)));
    }
#endif
  }

  // D-Extension
  if( feature->IsModeEnabled(RV_D) ){
    EnableExt(static_cast<RevExt *>(new RV32D(feature,RegFile,mem,output)),false);
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64D(feature,RegFile,mem,output)),false);
    }
  }

  // PAN Extension
  if( feature->IsModeEnabled(RV_P) ){
    EnableExt(static_cast<RevExt *>(new RV64P(feature,RegFile,mem,output)),false);
  }

  return true;
}

uint32_t RevProc::CompressCEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= Entry.opcode;
  Value |= uint32_t(Entry.funct2) << 2;
  Value |= uint32_t(Entry.funct3) << 4;
  Value |= uint32_t(Entry.funct4) << 8;
  Value |= uint32_t(Entry.funct6) << 12;

  return Value;
}

uint32_t RevProc::CompressEncoding(RevInstEntry Entry){
  uint32_t Value = 0x00;

  Value |= Entry.opcode;
  Value |= uint32_t(Entry.funct3)  << 8;
  Value |= uint32_t(Entry.funct7)  << 11;
  Value |= uint32_t(Entry.imm12)   << 18;
  Value |= uint32_t(Entry.fpcvtOp) << 30;  //this is a 5 bit field, but only the lower two bits are used, so it *just* fits
                                           //without going to a uint64

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
                    "Core %u ; Initializing table mapping for machine model=%s\n",
                    id, feature->GetMachineModel().c_str());

  for( unsigned i=0; i<InstTable.size(); i++ ){
    NameToEntry.insert(
      std::pair<std::string,unsigned>(ExtractMnemonic(InstTable[i]),i) );
    if( !InstTable[i].compressed ){
      // map normal instruction
      EncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %u ; Table Entry %u = %s\n",
                      id,
                      CompressEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }else{
      // map compressed instruction
      CEncToEntry.insert(
        std::pair<uint32_t,unsigned>(CompressCEncoding(InstTable[i]),i) );
      output->verbose(CALL_INFO, 6, 0,
                      "Core %u ; Compressed Table Entry %u = %s\n",
                      id,
                      CompressCEncoding(InstTable[i]),
                      ExtractMnemonic(InstTable[i]).c_str() );
    }
  }
  return true;
}

bool RevProc::ReadOverrideTables(){
  output->verbose(CALL_INFO, 6, 0,
                    "Core %u ; Reading override tables for machine model=%s\n",
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
    output->fatal(CALL_INFO, -1, "Error: failed to read instruction table for core=%u\n", id);

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
    RevRegFile* regFile = GetRegFile(t);
    regFile->RV32_PC = 0x00l;
    regFile->RV64_PC = 0x00ull;
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      regFile->RV32[i] = 0x00l;
      regFile->RV64[i] = 0x00ull;
      regFile->SPF[i]  = 0.f;
      regFile->DPF[i]  = 0.f;
      regFile->RV32_Scoreboard[i] = false;
      regFile->RV64_Scoreboard[i] = false;
      regFile->SPF_Scoreboard[i] = false;
      regFile->DPF_Scoreboard[i] = false;
    }

    // initialize all the relevant program registers
    // -- x2 : stack pointer
    regFile->RV32[2] = (uint32_t)(mem->GetStackTop());
    regFile->RV64[2] = mem->GetStackTop();

    // -- x3 : global pointer
    regFile->RV32[3] = (uint32_t)(loader->GetSymbolAddr("__global_pointer$"));
    regFile->RV64[3] = loader->GetSymbolAddr("__global_pointer$");

    // -- x8 : frame pointer
    regFile->RV32[8] = regFile->RV32[3];
    regFile->RV64[8] = regFile->RV64[3];

    regFile->cost = 0;

#if 0
    while(!Pipeline.empty()){
      Pipeline.pop();
    }
#endif
    Pipeline.clear();
  }
  // set the pc
  uint64_t StartAddr = 0x00ull;
  if( !opts->GetStartAddr( id, StartAddr ) )
    output->fatal(CALL_INFO, -1,
                  "Error: failed to init the start address for core=%u\n", id);
  std::string StartSymbol = "main";
  //std::string StartSymbol = "_start";
  if( StartAddr == 0x00ull ){
    if( !opts->GetStartSymbol( id, StartSymbol ) )
      output->fatal(CALL_INFO, -1,
                    "Error: failed to init the start symbol address for core=%u\n", id);

    StartAddr = loader->GetSymbolAddr(StartSymbol);
  }
  if( StartAddr == 0x00ull ){
    // load "main" symbol
    StartAddr = loader->GetSymbolAddr("main");
    //StartAddr = loader->GetSymbolAddr("_start");
    if( StartAddr == 0x00ull ){
      output->fatal(CALL_INFO, -1,
                    "Error: failed to auto discover address for <main> for core=%u\n", id);
    }
  }
  for (int t=0;  t < _REV_HART_COUNT_; t++){
    RevRegFile* regFile = GetRegFile(t);
    regFile->RV32_PC = (uint32_t)(StartAddr);
    regFile->RV64_PC = StartAddr;
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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs2     = ((Inst & 0b011100) >> 2);
  CompInst.rs1     = ((Inst & 0b01110000000) >> 7);

  // The immd is pre-scaled in this instruction format
  if(CompInst.funct3 == 0b110){
    //c.sw
    CompInst.imm     = ((Inst & 0b0100000) << 1);         //offset[6]
    CompInst.imm    |= ((Inst & 0b01110000000000) >> 6);   //offset[5:3]
    CompInst.imm    |= ((Inst & 0b01000000) >> 4);          //offset[2]
  }else{
    if(feature->GetXlen() == 32){
      //c.fsw
      CompInst.imm     = ((Inst & 0b00100000) << 1);         //imm[6]
      CompInst.imm     = ((Inst & 0b01000000) << 4);         //imm[2]
      CompInst.imm    |= ((Inst & 0b01110000000000) >> 7); //imm[5:3]
    }else{
      //c.sd
      CompInst.imm     = ((Inst & 0b01100000) << 1);         //imm[7:6]
      CompInst.imm    |= ((Inst & 0b01110000000000) >> 7); //imm[5:3]
    }
  }

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCAInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

  // encodings
  CompInst.opcode  = InstTable[Entry].opcode;
  CompInst.funct3  = InstTable[Entry].funct3;

  // registers
  CompInst.rs1     = ((Inst & 0b1110000000) >> 7);
  CompInst.offset  = ((Inst & 0b1111100) >> 2);
  CompInst.offset |= ((Inst & 0b1110000000000) >> 5);

  //swizzle: offset[8|4:3]  offset[7:6|2:1|5]
  std::bitset<16> tmp(0);
  // handle c.beqz/c.bnez offset
  if( (CompInst.opcode == 0b01) && (CompInst.funct3 >= 0b110) ){
    std::bitset<16> o(CompInst.offset);
    tmp[0] = o[1];
    tmp[1] = o[2];
    tmp[2] = o[5];
    tmp[3] = o[6];
    tmp[4] = o[0];
    tmp[5] = o[3];
    tmp[6] = o[4];
    tmp[7] = o[7];
  } else if( (CompInst.opcode == 0b01) && (CompInst.funct3 == 0b100)) {
    //We have a shift or a andi
    CompInst.rd = CompInst.rs1;
  }

  CompInst.offset = ((uint16_t)tmp.to_ulong()) << 1; // scale to corrrect position to be consistent with other compressed ops
  CompInst.imm = ((Inst & 0b01111100) >> 2);
  CompInst.imm |= ((Inst & 0b01000000000000) >> 7);


/*  // handle c.beqz/c.bnez offset
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
  }*/

  CompInst.instSize = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevProc::DecodeCJInst(uint16_t Inst, unsigned Entry){
  RevInst CompInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

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
  uint8_t opc     = 0;
  uint8_t funct2  = 0;
  uint8_t funct3  = 0;
  uint8_t funct4  = 0;
  uint8_t funct6  = 0;
  uint8_t l3      = 0;
  uint32_t Enc    = 0x00ul;
  uint64_t PC     = GetPC();
  RevInst TInst;

  if( !feature->HasCompressed() ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Compressed instructions not enabled!\n",
                  PC);

  }

  ResetInst(&TInst);

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
      uint8_t opSelect = ((TmpInst & 0b110000000000) >> 10);
      if( opSelect == 0b11 ){
        funct6 = ((TmpInst & 0b1111110000000000) >> 10);
        funct2 = ((TmpInst & 0b01100000) >> 5 );
      }else{
        funct3 = l3;
        funct2 = opSelect;
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

  bool isCoProcInst = false;
  std::map<uint32_t,unsigned>::iterator it = CEncToEntry.find(Enc);
  if( it == CEncToEntry.end() ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){
        //Create NOP - ADDI x0, x0 0
        uint8_t caddi_op= 0b01;
        Inst = 0;
        Enc = 0;
        Enc |= caddi_op;
        it = CEncToEntry.find(Enc);
      }else{
        output->fatal(CALL_INFO, -1,
                  "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%" PRIu32 "\n opc=%x; funct2=%x, funct3=%x, funct4=%x, funct6=%x\n",
                  PC,
                  Enc, opc, funct2, funct3, funct4, funct6 );
      }
  }

  unsigned Entry = it->second;
  if( Entry > (InstTable.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 "\
                  Opcode = %x Funct2 = %x Funct3 = %x Funct4 = %x Funct6 = %x Enc = %x \n", \
                  PC, opc, funct2, funct3, funct4, funct6, Enc );

  }

  RegFile->Entry = Entry;

  RegFile->trigger = false;

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

  // we should never arrive here
  // we return a null instruction in order to forego a compiler warning
  return TInst;
}

RevInst RevProc::DecodeRInst(uint32_t Inst, unsigned Entry){
  RevInst DInst;

  // cost
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RegFile->cost  = InstTable[Entry].cost;

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
  DInst.imm = ( (Inst >> 19) & 0b1000000000000 ) | // [12]
              ( (Inst <<  4) &  0b100000000000 ) | // [11]
              ( (Inst >> 20) &   0b11111100000 ) | // [10:5]
              ( (Inst >>  7) &         0b11110 ) ; // [4:1]

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
  RegFile->cost  = InstTable[Entry].cost;

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
  DInst.imm     = ( (Inst >> 11) & 0b100000000000000000000 ) | // imm[20]
                  ( (Inst)       &  0b11111111000000000000 ) | // imm[19:12]
                  ( (Inst >> 9)  &          0b100000000000 ) | // imm[11]
                  ( (Inst >> 20) &           0b11111111110 ) ; // imm[10:1]

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
  RegFile->cost  = InstTable[Entry].cost;

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
  RevRegFile* regFile = GetRegFile(HartToExec);
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    *Value = regFile->RV32[Idx];
    return true;
  }else{
    *Value = regFile->RV64[Idx];
    return true;
  }
}

bool RevProc::DebugWriteReg(unsigned Idx, uint64_t Value){
  RevRegFile* regFile = GetRegFile(HartToExec);
  if( !Halted )
    return false;
  if( Idx > (_REV_NUM_REGS_-1) ){
    return false;
  }
  if( feature->GetXlen() == 32 ){
    regFile->RV32[Idx] = (uint32_t)(Value&0xFFFFFFFF);
    return true;
  }else{
    regFile->RV64[Idx] = Value;
    return true;
  }
}

uint64_t RevProc::GetPC(){
  if( feature->GetXlen() == 32 ){
    return (uint64_t)(RegFile->RV32_PC);
  }else{
    return RegFile->RV64_PC;
  }
}

void RevProc::SetPC(uint64_t PC){
  if( feature->GetXlen() == 32 ){
    RegFile->RV32_PC = (uint32_t)(PC);
  }else{
    RegFile->RV64_PC = PC;
  }
}

bool RevProc::PrefetchInst(){
  uint64_t PC   = 0x00ull;
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile->RV32_PC);
  }else{
    PC = RegFile->RV64_PC;
  }

  // These are addresses that we can't decode
  // Return false back to the main program loop
  if( (PC == 0x00ull) ||
      (PC == _PAN_FWARE_JUMP_) ){
    return false;
  }

  return sfetch->IsAvail(PC);
}

RevInst RevProc::DecodeInst(){
  uint32_t Enc  = 0x00ul;
  uint32_t Inst = 0x00ul;
  uint64_t PC   = 0x00ull;
  bool Fetched  = false;
  RevInst TInst;

  ResetInst(&TInst);

  // Stage 1: Retrieve the instruction
  if( feature->GetXlen() == 32 ){
    PC = (uint64_t)(RegFile->RV32_PC);
  }else{
    PC = RegFile->RV64_PC;
  }

  if( !sfetch->InstFetch(PC, Fetched, Inst) ){
    output->fatal(CALL_INFO, -1,
                  "Error: failed to retrieve prefetched instruction at PC=0x%" PRIx64 "\n",
                  PC);
  }

  if(0 != Inst){
    output->verbose(CALL_INFO, 6, 0,
                    "Core %u ; Thread %d; PC:InstPayload = 0x%" PRIx64 ":0x%" PRIx32 "\n",
                    id, HartToDecode, PC, Inst);
  }else{
    output->fatal(CALL_INFO, -1,
                  "Error: Core %u failed to decode instruction at PC=0x%" PRIx64 "; Inst=%" PRIu32 "\n",
                  id,
                  PC,
                  Inst );
  }

  // Stage 1a: handle the crack fault injection
  if( CrackFault ){
    srand(time(NULL));
    uint64_t rval = rand() % (2^(fault_width));
    Inst |= rval;

    // clear the fault
    CrackFault = false;
  }

  // Stage 2: Retrieve the opcode
  const uint32_t Opcode = Inst & 0b1111111;

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
      //Atomics have a smaller funct7 field - trim out the aq and rl fields
      if(Opcode == 0b0101111){
          Funct7 = (Funct7 &0b01111100) >> 2;
      }
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

  uint32_t fcvtOp = 0;
  //Special encodings for FCVT instructions
  if( (0b1010011 == Opcode) && ((0b1100000 == Funct7) || (0b1101000 == Funct7)) ){
      fcvtOp =  DECODE_RS2(Inst);
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
  Enc |= (fcvtOp<<30);

  // Stage 7: Look up the value in the table
  bool isCoProcInst = false;
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
    Enc |= (fcvtOp<<30);
    it = EncToEntry.find(Enc);
    if( it == EncToEntry.end() ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){
        //Create NOP - ADDI x0, x0 0
        uint32_t addi_op= 0b0010011;
        Inst = 0;
        Enc = 0;
        Enc |= addi_op;
        it = EncToEntry.find(Enc);
      }else{
        // failed to decode the instruction
        output->fatal(CALL_INFO, -1,
                    "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%" PRIu32 "\n",
                    PC,
                    Enc );
      }
    }

  }

  unsigned Entry = it->second;

  if( Entry > (InstTable.size()-1) ){
      if(coProc){
        isCoProcInst = coProc->IssueInst(feature, RegFile, mem, Inst);
      }
      if(isCoProcInst){
        //Create NOP - ADDI x0, x0 0
        uint32_t addi_op= 0b0010011;
        Inst = 0;
        Enc = 0;
        Enc |= addi_op;
        it = EncToEntry.find(Enc);
        Entry = it->second;
      } else {
        output->fatal(CALL_INFO, -1,
                  "Error: no entry in table for instruction at PC=0x%" PRIx64 " \
                  Opcode = %x Funct3 = %x Funct7 = %x Imm12 = %x Enc = %x \n", \
                  PC, Opcode, Funct3, Funct7, Imm12, Enc );
      }

  }

  RegFile->Entry = Entry;

  RegFile->trigger = false;


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

  // we should never arrive here
  // we return a null instruction to forego a compiler warning
  return TInst;
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
  I->hazard     = nullptr;
}

void RevProc::HandleRegFault(unsigned width){
  // build the permissible set of registers available to fault
  unsigned LWidth = 0;
  std::vector<std::pair<std::string,void*>> RRegs;

  RevRegFile* regFile = GetRegFile(HartToExec);

  if( feature->GetXlen() == 32 ){
    if( width > feature->GetXlen() ){
      LWidth = feature->GetXlen();
    }else{
      LWidth = width;
    }

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->RV32[i])));
    }
  }else{
    // rv64
    LWidth = width;

    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "x" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->RV64[i])));
    }
  }

  if( feature->IsModeEnabled(RV_F) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->SPF[i])));
    }
  }else if( feature->IsModeEnabled(RV_D) ){
    for( unsigned i=0; i<_REV_NUM_REGS_; i++ ){
      std::string Name = "f" + std::to_string(i);
      RRegs.push_back( std::make_pair(Name,
                                      (void *)(&regFile->DPF[i])));
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
                  "FAULT:REG: Register fault of %u bits into register %s\n",
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

bool RevProc::DependencyCheck(uint16_t HartID, RevInst* I){

  bool depFound = false;
  bool isFloat = IsFloat(I->entry);

  RevRegFile* regFile = GetRegFile(HartID);

  // check the load hazard bit
  if( I->hazard != nullptr ){
    bool *Hazard = I->hazard;
    if( *Hazard ){
      return true;
    }
  }

  if(feature->IsRV32()){
    if(isFloat){
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->SPF_Scoreboard[I->rs3] || depFound : depFound;
    }else{
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->RV32_Scoreboard[I->rs3] || depFound : depFound;
    }
  }else {
    if(isFloat){
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->DPF_Scoreboard[I->rs3] || depFound : depFound;
    }else{
      depFound = (I->rs1 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs1] || depFound : depFound;
      depFound = (I->rs2 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs2] || depFound : depFound;
      depFound = (I->rs3 <= _REV_NUM_REGS_) ? regFile->RV64_Scoreboard[I->rs3] || depFound : depFound;
    }
  }
  return depFound;
}

void RevProc::DependencySet(uint16_t HartID, RevInst* Inst){
      RevRegFile* regFile = GetRegFile(HartID);
      if(Inst->rd != 0 && Inst->rd < _REV_NUM_REGS_){
        bool isFloat = IsFloat(Inst->entry);
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[Inst->rd] = true;
          }else{
            regFile->RV32_Scoreboard[Inst->rd] = true;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[Inst->rd] = true;
          }else{
            regFile->RV64_Scoreboard[Inst->rd] = true;
          }
      }
    }
}

void RevProc::DependencyClear(uint16_t HartID, RevInst* Inst){
    RevRegFile* regFile = GetRegFile(HartID);
    if(Inst->rd < _REV_NUM_REGS_){
        bool isFloat = IsFloat(Inst->entry);
        if(feature->IsRV32()){
          if(isFloat){
            regFile->SPF_Scoreboard[Inst->rd] = false;
          }else{
            regFile->RV32_Scoreboard[Inst->rd] = false;
          }
      }else{
          if(isFloat){
            regFile->DPF_Scoreboard[Inst->rd] = false;
          }else{
            regFile->RV64_Scoreboard[Inst->rd] = false;
          }
      }
    }
}

uint16_t RevProc::GetHartID(){
  if(HART_CTS.none()) { return HartToDecode;};

  uint16_t nextID = HartToDecode;
  if(HART_CTS[HartToDecode]){
    nextID = HartToDecode;
  }else{
    for(int tID = 0; tID < _REV_HART_COUNT_; tID++){
      nextID++;
      if(nextID >= _REV_HART_COUNT_){
        nextID = 0;
      }
      if(HART_CTS[nextID]){ break; };
    }
    output->verbose(CALL_INFO, 6, 0,
                    "Core %u ; Thread switch from %d to %d \n",
                    id, HartToDecode, nextID);
  }
  return nextID;
}

bool *RevProc::createLoadHazard(){
  bool *LH = new bool;
  *LH = false;
  LoadHazards.push_back(LH);
  return LH;
}

void RevProc::destroyLoadHazard(bool *LH){
  if( LH != nullptr ){
    LoadHazards.remove(LH);
  }
}

bool RevProc::ClockTick( SST::Cycle_t currentCycle ){
  bool rtn = false;
  Stats.totalCycles++;

#ifdef _REV_DEBUG_
  if((currentCycle % 100000000) == 0){
    std::cout << "Current Cycle: " << currentCycle <<  " PC: "
              << std::hex << ExecPC << std::dec << std::endl;
  }
#endif

  // -- MAIN PROGRAM LOOP --
  //
  // If the clock is down to zero, then fetch the next instruction
  // else if the the instruction has not yet been triggered, execute it
  // else, wait until the counter is decremented to zero to retire the instruction
  //
  //
  if( PendingCtxSwitch ){
    /*
     * There was a ctx switch event triggered
     * - Either a call to fork/clone
     * - Child process finished executing
     */
    if( Pipeline.empty() ) {
      if( !ChangeActivePID(NextPID) ){
        output->fatal(CALL_INFO, -1,
                      "Core %u ; Hart %u; PID %" PRIu32 " Failed to change active PID to %u\n",
                      id, HartToDecode, GetActivePID(), NextPID);
      } else {
        RegFile->trigger = 0;
        RegFile->cost = 0;
        ExecPC = GetPC();
        PendingCtxSwitch = false;
        NextPID = 0;
      }
    }
  }

  for (int tID = 0; tID < _REV_HART_COUNT_; tID++){
    HART_CTS[tID] = (GetRegFile(tID)->cost == 0);
  }

  if( HART_CTS.any() && (!Halted)) {
    // fetch the next instruction
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
      Inst.entry = RegFile->Entry;
    }

    Inst.hazard = nullptr;

    // Now that we have decoded the instruction, check for pipeline hazards
    if(Stalled || DependencyCheck(HartToDecode, &Inst)) {
      RegFile->cost = 0; // We failed dependency check, so set cost to 0 - this will
      Stats.cyclesIdle_Pipeline++;        // prevent the instruction from advancing to the next stage
      HART_CTE[HartToDecode] = false;
      HartToExec = _REV_INVALID_HART_ID_;
    }else {
      Stats.cyclesBusy++;
      HART_CTE[HartToDecode] = true;
      HartToExec = HartToDecode;
    }
    Inst.cost = RegFile->cost;
    Inst.entry = RegFile->Entry;
    rtn = true;
    ExecPC = GetPC();
  }

  if( ( (HartToExec != _REV_INVALID_HART_ID_) && !RegFile->trigger) && !Halted && HART_CTE[HartToExec]){
    // trigger the next instruction
    // HartToExec = HartToDecode;
    RegFile->trigger = true;

    // pull the PC
    output->verbose(CALL_INFO, 6, 0,
                    "Core %u ; Thread %d; Executing PC= 0x%" PRIx64 "\n",
                    id, HartToExec, ExecPC);

    // attempt to execute the instruction as long as it is NOT
    // the firmware jump PC
    if( ExecPC != _PAN_FWARE_JUMP_ ){

      // Find the instruction extension
      std::map<unsigned,std::pair<unsigned,unsigned>>::iterator it;
      it = EntryToExt.find(RegFile->Entry);
      if( it == EntryToExt.end() ){
        // failed to find the extension
        output->fatal(CALL_INFO, -1,
                    "Error: failed to find the instruction extension at PC=%" PRIx64 ".", ExecPC );
      }

      // found the instruction extension
      std::pair<unsigned,unsigned> EToE = it->second;
      RevExt *Ext = Extensions[EToE.first];

      // Update RegFile (in case of prior context switch)
      Ext->SetRegFile(RegFile);

      // -- BEGIN new pipelining implementation
      if( !PendingCtxSwitch ){
        bool *LH = createLoadHazard();
        Pipeline.push_back(std::make_pair(HartToExec,Inst));
        Pipeline.back().second.hazard = LH;
      }

      if( (Ext->GetName() == "RV32F") ||
          (Ext->GetName() == "RV32D") ||
          (Ext->GetName() == "RV64F") ||
          (Ext->GetName() == "RV64D") ){
        Stats.floatsExec++;
      }

      // set the hazarding
      DependencySet(HartToExec, &(Pipeline.back().second));
      // -- END new pipelining implementation

      // execute the instruction
      if( !Ext->Execute(EToE.second, Pipeline.back().second, HartToExec) ){
        output->fatal(CALL_INFO, -1,
                    "Error: failed to execute instruction at PC=%" PRIx64 ".", ExecPC );
      }

//      #define __REV_DEEP_TRACE__
      #ifdef __REV_DEEP_TRACE__
      if(feature->IsRV32()){
        std::cout << "RDT: Executed PC = " << std::hex << ExecPC
                  << " Inst: " << std::setw(23)
                  << InstTable[Inst.entry].mnemonic
                  << " r" << std::dec << (uint32_t)Inst.rd  << "= "
                  << std::hex << RegFile->RV32[Inst.rd]
                  << " r" << std::dec << (uint32_t)Inst.rs1 << "= "
                  << std::hex << RegFile->RV32[Inst.rs1]
                  << " r" << std::dec << (uint32_t)Inst.rs2 << "= "
                  << std::hex << RegFile->RV32[Inst.rs2]
                  << " imm = " << std::hex << Inst.imm
                  << std::endl;

      }else{
        std::cout << "RDT: Executed PC = " << std::hex << ExecPC \
                  << " Inst: " << std::setw(23)
                  << InstTable[Inst.entry].mnemonic
                  << " r" << std::dec << (uint32_t)Inst.rd  << "= "
                  << std::hex << RegFile->RV64[Inst.rd]
                  << " r" << std::dec << (uint32_t)Inst.rs1 << "= "
                  << std::hex << RegFile->RV64[Inst.rs1]
                  << " r" << std::dec << (uint32_t)Inst.rs2 << "= "
                  << std::hex << RegFile->RV64[Inst.rs2]
                  << " imm = " << std::hex << Inst.imm
                  << std::endl;
        std::cout << "RDT: Address of RD = 0x" << std::hex
                  << (uint64_t *)(&RegFile->RV64[Inst.rd])
                  << std::dec << std::endl;
      }
      #endif

      /*
       * Exception Handling
       * - Currently this is only for ecall
      */
      if( (RegFile->RV64_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ||
          (RegFile->RV32_SCAUSE == EXCEPTION_CAUSE::ECALL_USER_MODE) ){
        // Ecall found
        output->verbose(CALL_INFO, 6, 0,
                  "Core %u; HartID %d; PID %" PRIu32 " - Exception Raised: ECALL with code = %lu\n",
                  id, HartToExec, GetActivePID(), RegFile->RV64[17]);
        #ifdef _REV_DEBUG_
        //        std::cout << "Hart "<< HartToExec << " found ecall with code: "
        //                  << cRegFile->RV64[17] << std::endl;
        #endif

        /* Execute system call on this RevProc */
        ExecEcall();

        #ifdef _REV_DEBUG_
        //        std::cout << "Hart "<< HartToExec << " returned from ecall with code: "
        //        << rc << std::endl;
        #endif

        // } else {
        //   ExecEcall();
        #ifdef _REV_DEBUG_
        //        std::cout << "Hart "<< HartToExec << " found ecall with code: "
        //                  << code << std::endl;
        #endif

        /* exception handled... zero the cause registers */
        RegFile->RV64_SCAUSE = 0;
        RegFile->RV32_SCAUSE = 0;

        #ifdef _REV_DEBUG_
        //        std::cout << "Hart "<< HartToExec << " returned from ecall with code: "
        //                  << rc << std::endl;
        #endif
        // }
      }

      // inject the ALU fault
      if( ALUFault ){
        // inject ALU fault
        RevExt *Ext = Extensions[EToE.first];
        if( (Ext->GetName() == "RV32F") ||
            (Ext->GetName() == "RV32D") ){
          // write an rv32 float rd
          uint32_t rval = rand() % (2^(fault_width));
          uint32_t tmp = (uint32_t)(RegFile->SPF[Inst.rd]);
          tmp |= rval;
          RegFile->SPF[Inst.rd] = (float)(tmp);
        }else if( (Ext->GetName() == "RV64F") ||
                  (Ext->GetName() == "RV64D") ){
          // write an rv64 float rd
          uint64_t rval = rand() % (2^(fault_width));
          uint64_t tmp = (uint64_t)(RegFile->DPF[Inst.rd]);
          tmp |= rval;
          RegFile->DPF[Inst.rd] = (double)(tmp);
        }else if( feature->GetXlen() == 32 ){
          // write an rv32 gpr rd
          uint32_t rval = rand() % (2^(fault_width));
          RegFile->RV32[Inst.rd] |= rval;
        }else{
          // write an rv64 gpr rd
          uint64_t rval = rand() % (2^(fault_width));
          RegFile->RV64[Inst.rd] |= rval;
        }

        // clear the fault
        ALUFault = false;
      }
    }

    // if this is a singlestep, clear the singlestep and halt
    if( SingleStep ){
      SingleStep = false;
      Halted = true;
    }

    rtn = true;
  }else{
    // wait until the counter has been decremented
    // note that this will continue to occur until the counter is drained
    // and the HART is halted
    output->verbose(CALL_INFO, 9, 0,
                    "Core %u ; No available thread to exec PC= 0x%" PRIx64 "\n",
                    id, ExecPC);
    rtn = true;
    Stats.cyclesIdle_Total++;
    if(HART_CTE.any()){
      Stats.cyclesIdle_MemoryFetch++;
    }
  }

  // walk the pipeline hazards.  if a load hazard is set, increase the cost to > 0
  for( auto i : Pipeline ){
    if( *(i.second.hazard) ){
      i.second.cost++;
    }
  }

  // Check for pipeline hazards
  if(!Pipeline.empty() &&
     (Pipeline.front().second.cost > 0)){
    Pipeline.front().second.cost--;
    if((Pipeline.front().second.cost == 0) &&
       (!*(Pipeline.front().second.hazard))){
      // Ready to retire this instruction
      uint16_t tID = Pipeline.front().first;
      output->verbose(CALL_INFO, 6, 0,
                      "Core %u ; ThreadID %d; Retiring PC= 0x%" PRIx64 "\n",
                      id, tID, ExecPC);
      Retired++;
      DependencyClear(tID, &(Pipeline.front().second));
      destroyLoadHazard(Pipeline.front().second.hazard);
      Pipeline.erase(Pipeline.begin());
      GetRegFile(tID)->cost = 0;
    }else{
      // could not retire the instruction, bump the cost
      Pipeline.front().second.cost++;
    }
  }

  // Check for completion states and new tasks
  if( (GetPC() == _PAN_FWARE_JUMP_) || (GetPC() == 0x00ull) ){
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
                      "Core %u ; PAN Exec Jumping to PC= 0x%" PRIx64 "\n",
                      id, Addr);
          SetPC(Addr);
          done = false;
          break;
        case PanExec::QNull:
          // no work to do; spin on the firmware jump PC
          output->verbose(CALL_INFO, 6, 0,
                      "Core %u ; No PAN work to do; Jumping to PC= 0x%" PRIx64 "\n",
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
      done = true;
    }

    // determine if we have any outstanding memory requests
    if( mem->outstandingRqsts() ){
      done = false;
    }

    if( HartToExec != _REV_INVALID_HART_ID_ ){
      if( ActivePIDs.size() > HartToExec ) {
        uint32_t CurrPID = ActivePIDs.at(HartToExec);
        uint32_t ParentPID = ThreadTable.at(ActivePIDs.at(HartToExec))->GetParentPID();
        output->verbose(CALL_INFO, 2, 0,
                      "Thread %u completed execution.\n", CurrPID);
        if(ParentPID != 0 ){
          done = false;
          output->verbose(CALL_INFO, 2, 0,
                          "Switching from thread with PID = %u to its parent PID = %u\n",
                          ActivePIDs.at(HartToExec), ParentPID);
          CtxSwitchAlert(ParentPID);
          SwapToParent = true;
          ThreadTable.at(ActivePIDs.at(HartToExec))->SetState(ThreadState::Dead);
        } else {
          done = true;
        }
      }
    }
    if( done ){
      // we are really done, return
      output->verbose(CALL_INFO,2,0,"Program execution complete\n");
      Stats.percentEff = float(Stats.cyclesBusy)/Stats.totalCycles;
      output->verbose(CALL_INFO,2,0,
                      "Program Stats: Total Cycles: %" PRIu64 " Busy Cycles: %" PRIu64
                      " Idle Cycles: %" PRIu64 " Eff: %f\n",
                      Stats.totalCycles, Stats.cyclesBusy,
                      Stats.cyclesIdle_Total, static_cast<double>(Stats.percentEff));
      output->verbose(CALL_INFO,3,0,"\t Bytes Read: %" PRIu32 " Bytes Written: %" PRIu32
                      " Floats Read: %" PRIu32 " Doubles Read %" PRIu32 " Floats Exec: %" PRIu64
                      " TLB Hits: %" PRIu64 " TLB Misses: %" PRIu64 " Inst Retired: %" PRIu64 "\n",
                      mem->memStats.bytesRead,
                      mem->memStats.bytesWritten,
                      mem->memStats.floatsRead,
                      mem->memStats.doublesRead,
                      Stats.floatsExec,
                      mem->memStats.TLBHits,
                      mem->memStats.TLBMisses,
                      Retired);
      return false;
    }
  }

  return rtn;
}


/* System Call & Thread Stuff Below */
uint32_t RevProc::HartToExecPID(){
  if( ActivePIDs.size() <= HartToExec )
    return ActivePIDs.at(HartToExec);
  else{
    return 0;
  }
}

std::shared_ptr<RevThreadCtx> RevProc::HartToExecCtx(){
  if( HartToExec <= ActivePIDs.size() )
    return ThreadTable.at(ActivePIDs.at(HartToExec));
  else{
    return 0;
  }
}


uint32_t RevProc::HartToDecodePID(){
  if( ActivePIDs.size() <= HartToDecode )
    return ActivePIDs.at(HartToDecode);
  else{
    output->fatal(CALL_INFO, -1,
                  "Tried to get active PID for HartToDecode = %d but there is no ActivePID for that Hart\n",
                  HartToExec);
    return 1;
  }
}

bool RevProc::UpdateRegFile(){
  uint16_t HartID = GetHartID();
  auto it = ThreadTable.find(ActivePIDs.at(HartID));
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> Ctx = it->second;
    RegFile = Ctx->GetRegFile();
    return true;
  }
  else {
    output->fatal(CALL_INFO, -1,
                  "Failed to find RegFile for PID = %u on Hart = %u \n", ActivePIDs.at(HartID), HartID);
  }
  return false;
}


RevRegFile* RevProc::GetRegFile(uint16_t HartID){
  auto it = ThreadTable.find(ActivePIDs.at(HartID));
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> Ctx = it->second;
    return Ctx->GetRegFile();
  }
  else {
    output->fatal(CALL_INFO, -1,
                  "Failed to find RegFile for PID = %u on Hart = %u \n", ActivePIDs.at(HartID), HartID);
  }
  return 0;
}
//

bool RevProc::InitThreadTable(){
  /*
   * We need to create the first Ctx for each HART which will have the following attributes:
   * - PID = 1024 + However many already initialized Ctx objects there are
   * - ParentPID = 0 : (Only the first thread on every RevProc has ParentPID = 0)
   * - MemStartAddr : Top of stack (NOTE: No functionality yet)
   * - MemStartSize : _DEFAULT_THREAD_MEM_SIZE_ (NOTE: No functionality yet)
  */

  for( unsigned HartID=0; HartID<_REV_HART_COUNT_; HartID++){
    uint32_t ParentPID = 0;
    uint32_t FirstActivePID = mem->GetNewThreadPID();

    std::shared_ptr<RevThreadCtx> DefaultCtx = std::make_shared<RevThreadCtx>(
        FirstActivePID,
        ParentPID);

    /* Set the first RegFile as ActiveRegFile */
    RegFile = DefaultCtx->GetRegFile();

    /* Add first PID to ActivePIDs */
    ActivePIDs.emplace_back(FirstActivePID);

    /* Add to ThreadTable */
    ThreadTable.emplace(FirstActivePID, DefaultCtx);
  }
  return true;
}


/* =====================================================
 * ChangeActivePID(NewPID)
 * =====================================================
 * This function changes the active pid of HartToExec
 *
 * Returns:
 * - True if successfully changed
 * - False if not (ie. PID doesn't exist)
 *
 * NOTES:
 * - This function automatically sets the new Ctx state to Running
 * - This function automatically sets old Ctx state to Waiting
 */
bool RevProc::ChangeActivePID(uint32_t NewPID){
  auto it = ThreadTable.find(NewPID);
  if( it != ThreadTable.end() ){
    std::shared_ptr<RevThreadCtx> NewCtx = it->second;
    /* If switching to parent, output the child is being removed from the ThreadTable */
    if( SwapToParent ){
      output->verbose(CALL_INFO, 2, 0, "Removing ThreadCtx w/ PID = %u from the ThreadTable\n",
                      ActivePIDs.at(HartToExec));
      ThreadTable.erase(ActivePIDs.at(HartToExec));
    }
    ActivePIDs.at(HartToExec) = NewPID;
    ActivePIDs.at(HartToDecode) = NewPID;
    UpdateRegFile();
    return true;
  }else{
    /* TODO: Maybe don't output fatal? */
    output->fatal(CALL_INFO, -1,
                  "Failed to load ctx w/ PID=%u into Hart=%d because PID does not exist in ThreadTable\n",
                  NewPID, HartToExec);
    return false;
  }
}

/* NOTE: This is currently not used but will once more complex scheduling is supported */
/* ChangeActivePID(PID, HartID)
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
      output->fatal(CALL_INFO, -1, "Failed to load ctx w/ PID=%u into Hart=%u because Hart does not exist",
                    PID, HartToExec);
      return false;
    }
  }else{
    /* TODO: Maybe don't output fatal? */
    output->fatal(CALL_INFO, -1,
                  "Failed to load ctx w/ PID=%u into Hart=%u because PID does not exist in ThreadTable",
                  PID, HartToExec);
    return false;
  }
}

/* Returns vector of all PIDs in the ThreadTable */
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
*/
uint32_t RevProc::CreateChildCtx() {
  /* We get the currently executing PID's context as this is assumed to be the parent */
  std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

  /* Get new PID from global counter in RevMem */
  uint32_t ChildPID = mem->GetNewThreadPID();

  /* Create ChildCtx as a copy of ParentCtx */
  auto ChildCtx = std::make_shared<RevThreadCtx>(ChildPID,
                                       ActivePIDs.at(HartToExec));

  /* Child's Regfile is the same as the parent's with the exception of return value */
  ChildCtx->DuplicateRegFile(*RegFile);

  /* Add child to Proc's ThreadTable */
  ThreadTable.emplace(ChildPID, ChildCtx);

  /* Get Child's regfile so we can make the below modifications */
  RevRegFile* ChildRegFile = ChildCtx->GetRegFile();

  /* Zero the childs cause registers as they have no exceptions raised */
  ChildRegFile->RV64_SCAUSE = 0;
  ChildRegFile->RV32_SCAUSE = 0;

  /* The child's return value from fork/clone is 0 */
  ChildRegFile->RV64[10] = 0;

  /* Add ChildPID to list of Parent's Children */
  ParentCtx->AddChildPID(ChildPID); /* NOTE: This has no functionality at this point */

  return ChildPID;
}


/* ========================================= */
/* System Call (ecall) Implementations Below */
/* ========================================= */
void RevProc::InitEcallTable(){
  Ecalls = {
    {5,   &RevProc::ECALL_setxattr},
    {17,  &RevProc::ECALL_getcwd},          // Not implemented
    {23,  &RevProc::ECALL_dup},             // Not implemented
    {24,  &RevProc::ECALL_dup3},            // Not implemented
    {34,  &RevProc::ECALL_mkdirat},
    {49,  &RevProc::ECALL_chdir},
    {54,  &RevProc::ECALL_fchownat},        // Not implemented
    {55,  &RevProc::ECALL_fchown},          // Not implemented
    {56,  &RevProc::ECALL_openat},
    {57,  &RevProc::ECALL_close},           // Not implemented
    {63,  &RevProc::ECALL_read},            // Not implemented
    {64,  &RevProc::ECALL_write},
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
    {178, &RevProc::ECALL_gettid},
    {214, &RevProc::ECALL_brk},             // Not implemented
    {215, &RevProc::ECALL_munmap},
    {220, &RevProc::ECALL_clone},           // Fork functionality works but not clone3
    {222, &RevProc::ECALL_mmap},            //
    {403, &RevProc::ECALL_clock_gettime},   // Not implemented
    {404, &RevProc::ECALL_clock_settime},   // Not implemented
    {408, &RevProc::ECALL_timer_gettime},   // Not implemented
    {409, &RevProc::ECALL_timer_settime},   // Not implemented
    };
}

/* ======================================================= */
/* rev_setxattr(const char *path, const char *name,        */
/*              const void *value, size_t size, int flags) */
/*======================================================== */
void RevProc::ECALL_setxattr(){
  const char *path = reinterpret_cast<char*>(RegFile->RV64[10]);
  const char *name = reinterpret_cast<char*>(RegFile->RV64[11]);
  const void *value = reinterpret_cast<void*>(RegFile->RV64[12]);
  size_t size = RegFile->RV64[13];
  uint64_t flags = RegFile->RV64[14];

#ifdef __APPLE__
  uint32_t position = 0;
  uint64_t rc = setxattr(path, name, value, size, position, flags);
#else
  uint64_t rc = setxattr(path, name, value, size, flags);
#endif
  RegFile->RV64[10] = rc;
  return;
}

/* Increments program break by n bytes  */
void RevProc::ECALL_brk(){
  uint64_t Addr = RegFile->RV64[10];

  const uint64_t heapend = mem->GetHeapEnd();
  if( Addr > 0 && Addr > heapend ){
    uint64_t Size = Addr - heapend;
    mem->ExpandHeap(Size);
  } else {
    output->fatal(CALL_INFO, 11,
                  "Out of memory / Unable to expand system break (brk) to Addr = 0x%lx", Addr);
  }
  return;
}

/* ======================================================= */
/* rev_clone3(struct clone_args*, size_t args_size)        */
/* ======================================================= */
void RevProc::ECALL_clone(){
  uint64_t CloneArgsAddr = RegFile->RV64[10];
  // size_t SizeOfCloneArgs = RegFile()->RV64[11];

  /* Fetch the clone_args */
  struct clone_args args;
  mem->ReadMem(CloneArgsAddr, sizeof(uint64_t), &args);

  /*
   * Parse clone flags
   * NOTE: if no flags are set, we get fork() like behavior
   */
  for( uint64_t bit=1; bit != 0; bit <<= 1 ){
    switch (args.flags & bit) {
      case CLONE_VM:
        // std::cout << "CLONE_VM is true" << std::endl;
        break;
      case CLONE_FS: /* Set if fs info shared between processes */
        // std::cout << "CLONE_FS is true" << std::endl;
        break;
      case CLONE_FILES: /* Set if open files shared between processes */
        // std::cout << "CLONE_FILES is true" << std::endl;
        break;
      case CLONE_SIGHAND: /* Set if signal handlers shared */
        // std::cout << "CLONE_SIGHAND is true" << std::endl;
        break;
      case CLONE_PIDFD: /* Set if a pidfd should be placed in the parent */
        // std::cout << "CLONE_PIDFD is true" << std::endl;
        break;
      case CLONE_PTRACE: /* Set if tracing continues on the child */
        // std::cout << "CLONE_PTRACE is true" << std::endl;
        break;
      case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
        // std::cout << "CLONE_VFORK is true" << std::endl;
        break;
      case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
        // std::cout << "CLONE_PARENT is true" << std::endl;
        break;
      case CLONE_THREAD: /* Set to add to same thread group */
        // std::cout << "CLONE_THREAD is true" << std::endl;
        break;
      case CLONE_NEWNS: /* Set to create new namespace */
        // std::cout << "CLONE_NEWNS is true" << std::endl;
        break;
      case CLONE_SYSVSEM: /* Set to shared SVID SEM_UNDO semantics */
        // std::cout << "CLONE_SYSVSEM is true" << std::endl;
        break;
      case CLONE_SETTLS: /* Set TLS info */
        // std::cout << "CLONE_SETTLS is true" << std::endl;
        break;
      case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
        // std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
        break;
      case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
        // std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
        break;
      case CLONE_DETACHED: /* Create clone detached */
        // std::cout << "CLONE_DETACHED is true" << std::endl;
        break;
      case CLONE_UNTRACED: /* Set if the tracing process can't force CLONE_PTRACE on this clone */
        // std::cout << "CLONE_UNTRACED is true" << std::endl;
        break;
      case CLONE_CHILD_SETTID: /* New cgroup namespace */
        // std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
        break;
      case CLONE_NEWCGROUP: /* New cgroup namespace */
        // std::cout << "CLONE_NEWCGROUP is true" << std::endl;
        break;
      case CLONE_NEWUTS: /* New utsname group */
        // std::cout << "CLONE_NEWUTS is true" << std::endl;
        break;
      case CLONE_NEWIPC: /* New ipcs */
        // std::cout << "CLONE_NEWIPC is true" << std::endl;
        break;
      case CLONE_NEWUSER: /* New user namespace */
        // std::cout << "CLONE_NEWUSER is true" << std::endl;
        break;
      case CLONE_NEWPID: /* New pid namespace */
        // std::cout << "CLONE_NEWPID is true" << std::endl;
        break;
      case CLONE_NEWNET: /* New network namespace */
        // std::cout << "CLONE_NEWNET is true" << std::endl;
        break;
      case CLONE_IO: /* Clone I/O Context */
        // std::cout << "CLONE_IO is true" << std::endl;
        break;
      default:
        break;
    }
  }

  /* Get the parent ctx (Current active, executing PID) */
  std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

  /* Create the child ctx */
  uint32_t ChildPID = CreateChildCtx();
  std::shared_ptr<RevThreadCtx> ChildCtx = ThreadTable.at(ChildPID);

  /* TODO: Create a copy of Parents Memory Space (need Demand Paging first) */

  /*
   * ===========================================================================================
   * Register File
   * ===========================================================================================
   * We need to duplicate the parent's RegFile to to the Childs
   * - NOTE: when we return from this function, the return value will
   *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
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

  /*
   Alert the Proc there needs to be a Ctx switch
   Pass the PID that will be switched to once the
   current pipeline is executed until completion
  */
  CtxSwitchAlert(ChildPID);

  /* Parent's return value is the child's PID */
  RegFile->RV64[10] = ChildPID;

  /* Child's return value is 0 */
  ChildCtx->GetRegFile()->RV64[10] = 0;

  return;
}


/* =============================== */
/* rev_chdir(const char *filename) */
/* =============================== */
void RevProc::ECALL_chdir(){
  output->verbose(CALL_INFO, 2, 0, "ECALL_chdir called\n");
  std::string path = "";
  unsigned i=0;

  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    mem->ReadMem(RegFile->RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = chdir(path.data());
  RegFile->RV64[10] = rc;
}

/* ============================================================ */
/* rev_mkdirat(int dfd, const char * path, unsigned short mode) */
/* ============================================================ */
// void RevProc::ECALL_mkdir(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL_mkdir called\n");
//   std::string path = "";
//   unsigned i=0;

//   // we don't know how long the path string is so read a byte (char)
//   // at a time and search for the string terminator character '\0'
//   do {
//     char dirchar;
//     mem->ReadMem(RegFile->RV64[10] + sizeof(char)*i, sizeof(char), &dirchar);
//     path = path + dirchar;
//     i++;
//   } while( path.back() != '\0');

//   const int rc = chdir(path.data());
//   RegFile->RV64[10] = rc;
// }



/* ======================== */
/* rev_exit(int error_code) */
/* ======================== */
void RevProc::ECALL_exit(){
  output->verbose(CALL_INFO, 2, 0, "ECALL_exit called\n");
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();
  const uint64_t status = RegFile->RV64[10];

  /* If the current ctx has ParentPID = 0,
     it has no parent and we should terminate the sim */
  if( CurrCtx->GetParentPID() == 0 ){
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
    exit(status);
  } else {
    /* Parent exists & Child is exiting... switch back to parent */
    CtxSwitchAlert(CurrCtx->GetParentPID());
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
    return;
  }
  return;
}

/* ========================================= */
/* rev_getcwd(char *buf, unsigned long size) */
/* ========================================= */
void RevProc::ECALL_getcwd(){
  uint64_t BufAddr = RegFile->RV64[10];
  uint64_t size = RegFile->RV64[11];
  std::string CWD = std::filesystem::current_path().c_str();
  mem->WriteMem(feature->GetHart(), BufAddr, size, &CWD);

  /* Returns null-terminated string in buf */
  RegFile->RV64[10] = BufAddr;

  return;
}

/* ================ */
/* rev_getpid(void) */
/* ================ */
void RevProc::ECALL_getpid(){
  /* TODO: Implement error handling */
  output->verbose(CALL_INFO, 2, 0, "ECALL_getpid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  RegFile->RV64[10] = ActivePIDs.at(HartToExec);
  return;
}

/* ================= */
/* rev_getppid(void) */
/* ================= */
void RevProc::ECALL_getppid(){
/* TODO: Implement error handling */
  output->verbose(CALL_INFO, 2, 0, "ECALL_getppid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  uint32_t ParentPID = CurrentCtx->GetParentPID();
  RegFile->RV64[10] = ParentPID;
  return;
}

/* ========================================================== */
/* rev_write(unsigned int fd, const char *buf, size_t nbytes) */
/* ========================================================== */
void RevProc::ECALL_write(){
  output->verbose(CALL_INFO, 2, 0, "ECALL_write called\n");
  int fildes = RegFile->RV64[10];
  size_t nbytes = RegFile->RV64[12];

  auto buf = std::vector<char>(nbytes);
  char bufchar;
  for (size_t i=0; i<nbytes; i++){
  mem->ReadMem(RegFile->RV64[11] + sizeof(char)*i, sizeof(char), &bufchar);
  }
  mem->ReadMem(RegFile->RV64[11], nbytes, &buf[0]);

  /* Perform the write on the host system */
  const int rc = write(fildes, &buf[0], nbytes);

  /* write returns the number of bytes written */
  RegFile->RV64[10] = rc;
}


/* ========================================================================== */
/* rev_timer_settime(timer_t timer_id, int flags, */
/*   const struct __kernel_itimerspec  *new_setting, */
/*   struct __kernel_itimerspec  *old_setting) */
/* ========================================================================== */
void RevProc::ECALL_timer_settime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called\n");
  return;
}


/* ======================================================================== */
/* rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *setting) */
/* ======================================================================== */
void RevProc::ECALL_timer_gettime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called\n");
  return;
}

/* ========================================================================== */
/* rev_clock_settime(clockid_t which_clock, */
/* const struct __kernel_timespec *tp) */
/* ========================================================================== */
void RevProc::ECALL_clock_settime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called\n");
  return;
}

/* ============================================================ */
/* rev_clock_gettime(clockid_t which_clock, struct timeval *tp) */
/* ============================================================ */
void RevProc::ECALL_clock_gettime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called\n");
  return;
}


/* ====================================== */
/* rev_mmap(struct mmap_arg_struct *args) */
/* ====================================== */
// void *mmap(void *addr, size_t length, int prot, int flags,
//          int fd, off_t offset);
void RevProc::ECALL_mmap(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mmap called\n");

  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];
  // uint64_t Prot = RegFile->RV64[12];
  // uint64_t Flags = RegFile->RV64[13];
  // uint64_t fd = RegFile->RV64[14];
  // uint64_t offset = RegFile->RV64[15];

  if( !Addr ){
    // If address is NULL... We add it to MemSegs.end()->getTopAddr()+1
    Addr = mem->AllocMem(Size+1);
    // Addr = mem->AddMemSeg(Size);
  } else {
    // We were passed an address... try to put a segment there.
    // Currently there is no handling of getting it 'close' to the
    // suggested address... instead if it can't allocate a new segment
    // there it fails.
    if( !mem->AddMemSeg(Addr, Size) ){
      output->fatal(CALL_INFO, 11, "Failed to add mem segment\n");
    }
  }
  // std::cout << "MMAP Returning Addr = 0x" << Addr << std::endl;
  RegFile->RV64[10] = Addr;
  return;
}

/* ================================== */
/* munmap(void *addr, size_t length); */
/* ================================== */
void RevProc::ECALL_munmap(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munmap called\n");
  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];

  if( !mem->DeallocMem(Addr, Size) ){
    output->fatal(CALL_INFO, 11,
                  "Failed to perform munmap(Addr = 0x%lx, Size = 0x%lx)",
                  Addr, Size);
  }
  return;
}


/* ================ */
/* rev_gettid(void) */
/* ================ */
void RevProc::ECALL_gettid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettid called\n");
  RevRegFile* regFile = RegFile;

  /* rc = Currently Executing Hart */
  regFile->RV64[10] = HartToExec;
  return;
}

/* ========================================================= */
/* rev_settimeofday(struct timeval *tv, struct timezone *tz) */
/* ========================================================= */
void RevProc::ECALL_settimeofday(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: settimeofday called\n");
  return;
}

/* ============================================================= */
/* int rev_gettimeofday(struct timeval *tv, struct timezone *tz) */
/* ============================================================= */
void RevProc::ECALL_gettimeofday(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettimeofday called\n");
  return;
}


/* ========================================================================== */
/* rev_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset, */
/* size_t sigsetsize) */
/* ========================================================================== */
void RevProc::ECALL_rt_sigprocmask(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigprocmask called\n");
  return;
}

/* ================================== */
/* rev_timer_delete(timer_t timer_id) */
/* ================================== */
void RevProc::ECALL_timer_delete(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_delete called\n");
  return;
}

/* ===========================================================================*/
/* rev_timer_create(clockid_t which_clock, struct sigevent *timer_event_spec, */
/*   timer_t *created_timer_id) */
/* ===========================================================================*/
void RevProc::ECALL_timer_create(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_create called\n");
  return;
}


/* ========================================================================== */
/* rev_nanosleep(struct __kernel_timespec *rqtp, */
/* struct __kernel_timespec *rmtp) */
/* ========================================================================== */
void RevProc::ECALL_nanosleep(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: nanosleep called\n");
  return;
}

/* ========================================================================== */
/* rev_get_robust_list(int pid, struct robust_list_head *head_ptr, */
/*   size_t *len_ptr) */
/* ========================================================================== */
void RevProc::ECALL_get_robust_list(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_robust_list called\n");
  return;
}

/* ============================================================== */
/* rev_set_robust_list(struct robust_list_head *head, size_t len) */
/* ============================================================== */
void RevProc::ECALL_set_robust_list(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_robust_list called\n");
  return;
}

/* ========================================================================== */
/* rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, */
/*   struct rusage  *ru) */
/* ==========================================================================*/
void RevProc::ECALL_waitid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: waitid called\n");
  return;
}

/* ============================== */
/* rev_exit_group(int error_code) */
/* ============================== */
void RevProc::ECALL_exit_group(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit_group called\n");
  return;
}

/* ============================== */
/* rev_fdatasync(unsigned int fd) */
/* ============================== */
void RevProc::ECALL_fdatasync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fdatasync called\n");
  return;
}

/* ========================== */
/* rev_fsync(unsigned int fd) */
/* ========================== */
void RevProc::ECALL_fsync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsync called\n");
  return;
}

/* ============== */
/* rev_sync(void) */
/* ============== */
void RevProc::ECALL_sync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync called\n");
  return;
}

/* =================================================================== */
/*  ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags) */
/* =================================================================== */
void RevProc::ECALL_tee(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tee called\n");
#if 0
  // commented out to remove warnings
  int fd_in      = RegFile->RV64[10];
  int fd_out     = RegFile->RV64[11];
  size_t len     = RegFile->RV64[12];
  uint64_t flags = RegFile->RV64[13];
#endif
  return;
}

/* =================================================================== */
/* int openat(int dirfd, const char *pathname, int flags, mode_t mode) */
/* =================================================================== */
void RevProc::ECALL_openat(){
  int dfd = RegFile->RV64[10];
  int filenameAddr = RegFile->RV64[11];
  int flags = RegFile->RV64[12]; /* NOTE: Unused for now */
  uint64_t mode = RegFile->RV64[13];

  /*
   * NOTE: this is currently only opening files in the current directory
   *       because of some oddities in parsing the arguments & flags
   *       but this will be fixed in the near future
  */


  /* Read the filename from memory one character at a time until we find '\0' */
  std::string filename = "";
  unsigned i = 0;
  do {
    char filenameChar;
    mem->ReadMem(filenameAddr + sizeof(char)*i, sizeof(char), &filenameChar);
    filename = filename + filenameChar;
    i++;
  } while( filename.back() != '\0');

  dfd = open(std::filesystem::current_path().c_str(), O_RDONLY);

  /* Do the openat on the host */
  int fd = openat(dfd, filename.c_str(), O_RDWR);

  HartToExecCtx()->AddFD(fd);

  /* openat returns the file descriptor of the opened file */
  RegFile->RV64[10] = fd;
  return;
}

/* =================================================== */
/* rev_read(unsigned int fd, char *buf, size_t nbytes) */
/* =================================================== */
void RevProc::ECALL_read(){
  int fd = RegFile->RV64[10];
  uint64_t BufAddr = RegFile->RV64[11];
  size_t BufSize = RegFile->RV64[12];

  /* Check if Current Ctx has access to the fd */
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();

  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %u; Hart %" PRIu16 "; PID %" PRIu32
                  " tried to read from file descriptor: %d, but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
    return;
  }
  /*
   * This buffer is an intermediate buffer for storing the data read from host
   * for later use in writing to RevMem
  */
  char TmpBuf[BufSize];

  /*
   * Read nbytes of fd from host
   *
   * NOTE: Because the fd is in the Ctx's fildes vector, we can reasonably
   *       assume the file is already open on the host system because we
   *       try to maintain parity between those
   */

  /* Do the read on the host */
  uint64_t rc = read(fd, &TmpBuf, BufSize);

  /* Write that data to the buffer inside of Rev */
  mem->WriteMem(feature->GetHart(), BufAddr, BufSize, &TmpBuf);

  RegFile->RV64[10] = rc;
  return;
}


/* ========================== */
/* rev_close(unsigned int fd) */
/* ========================== */
void RevProc::ECALL_close(){
  int fd = RegFile->RV64[10];
  std::shared_ptr<RevThreadCtx> CurrCtx = HartToExecCtx();

  /* Check if CurrCtx has fd in fildes vector */
  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %u; Hart %d; PID %" PRIu32 " tried to close file descriptor %d but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
    return;
  }
  /* Close file on host */
  uint64_t rc = close(fd);

  /* Remove from Ctx's fildes */
  CurrCtx->RemoveFD(fd);

  /* rc is propogated to rev from host */
  RegFile->RV64[10] = rc;

  return;
}

/* ====================================================================== */
/* rev_fchown(unsigned int fd, unsigned short user, unsigned short group) */
/* ====================================================================== */
void RevProc::ECALL_fchown(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchown called");
  return;
}

/* ==================================================================================== */
/* rev_fchownat(int dfd, const char *filename, unsigned user, unsigned group, int flag) */
/* ==================================================================================== */
void RevProc::ECALL_fchownat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchownat called");
  return;
}

/* ======================================================================*/
/* rev_mkdirat(int dfd, const char * path, unsigned short mode)          */
/* ======================================================================*/
void RevProc::ECALL_mkdirat(){

  output->verbose(CALL_INFO, 2, 0, "ECALL_mkdirat called");

  unsigned fd = RegFile->RV64[10];
  unsigned Mode = RegFile->RV64[12];

  std::string path = "";
  unsigned i=0;

  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    mem->ReadMem(RegFile->RV64[11] + sizeof(char)*i, sizeof(char), &dirchar);
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = mkdirat(fd, path.data(), Mode);
  RegFile->RV64[10] = rc;
  return;
}

/* =========================================================== */
/* rev_dup3(unsigned int oldfd, unsigned int newfd, int flags) */
/* =========================================================== */
void RevProc::ECALL_dup3(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup3 called");
  return;
}


/* =========================================================== */
/* rev_dup(unsigned int fildes)                                */
/* =========================================================== */
void RevProc::ECALL_dup(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup called");
  return;
}


/*
 * This is the function that is called when an ECALL exception is detected inside ClockTick
 * - Currently the only way to set this exception is by Ext->Execute(....) an ECALL instruction
 *
 * Eventually this will be integrated into a TrapHandler however since ECALLs are the only
 * supported exceptions at this point there is no need just yet.
 */
void RevProc::ExecEcall(){
  // a7 register = ecall code
  uint64_t EcallCode;
  if( feature->IsRV32() )
    EcallCode = RegFile->RV32[17];
  else if( feature->IsRV64() )
    EcallCode = RegFile->RV64[17];
  else {
    return;
  }

  auto it = Ecalls.find(EcallCode);
  if( it != Ecalls.end() ){
    /* call the function */
    (it->second)(this);
    /* Trap handled... 0 cause registers */
    RegFile->RV64_SCAUSE = 0;
    RegFile->RV32_SCAUSE = 0;
  } else {
    output->fatal(CALL_INFO, -1, "Ecall Code = %lu not found", EcallCode);
  }
}

// EOF
// EOF
