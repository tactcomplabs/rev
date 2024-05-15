//
// _RevCore_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevCore.h"
#include "RevSysCalls.cc"
#include "sst/core/output.h"

namespace SST::RevCPU {

using MemSegment = RevMem::MemSegment;

RevCore::RevCore(
  unsigned                  Id,
  RevOpts*                  Opts,
  unsigned                  NumHarts,
  RevMem*                   Mem,
  RevLoader*                Loader,
  std::function<uint32_t()> GetNewTID,
  SST::Output*              Output
)
  : Halted( false ), Stalled( false ), SingleStep( false ), CrackFault( false ), ALUFault( false ), fault_width( 0 ), id( Id ),
    HartToDecodeID( 0 ), HartToExecID( 0 ), numHarts( NumHarts ), opts( Opts ), mem( Mem ), coProc( nullptr ), loader( Loader ),
    GetNewThreadID( std::move( GetNewTID ) ), output( Output ), feature( nullptr ), sfetch( nullptr ), Tracer( nullptr ) {

  // initialize the machine model for the target core
  std::string Machine;
  if( !Opts->GetMachineModel( id, Machine ) )
    output->fatal( CALL_INFO, -1, "Error: failed to retrieve the machine model for core=%" PRIu32 "\n", id );

  unsigned MinCost = 0;
  unsigned MaxCost = 0;

  Opts->GetMemCost( Id, MinCost, MaxCost );

  LSQueue = std::make_shared<std::unordered_multimap<uint64_t, MemReq>>();
  LSQueue->clear();

  // Create the Hart Objects
  for( size_t i = 0; i < numHarts; i++ ) {
    Harts.emplace_back( std::make_unique<RevHart>( i, LSQueue, [=]( const MemReq& req ) { this->MarkLoadComplete( req ); } ) );
    ValidHarts.set( i, true );
  }

  featureUP = std::make_unique<RevFeature>( Machine, output, MinCost, MaxCost, Id );
  feature   = featureUP.get();
  if( !feature )
    output->fatal( CALL_INFO, -1, "Error: failed to create the RevFeature object for core=%" PRIu32 "\n", id );

  unsigned Depth = 0;
  Opts->GetPrefetchDepth( Id, Depth );
  if( Depth == 0 ) {
    Depth = 16;
  }

  sfetch =
    std::make_unique<RevPrefetcher>( Mem, feature, Depth, LSQueue, [=]( const MemReq& req ) { this->MarkLoadComplete( req ); } );
  if( !sfetch )
    output->fatal( CALL_INFO, -1, "Error: failed to create the RevPrefetcher object for core=%" PRIu32 "\n", id );

  // load the instruction tables
  if( !LoadInstructionTable() )
    output->fatal( CALL_INFO, -1, "Error : failed to load instruction table for core=%" PRIu32 "\n", id );

  // reset the core
  if( !Reset() )
    output->fatal( CALL_INFO, -1, "Error: failed to reset the core resources for core=%" PRIu32 "\n", id );
}

bool RevCore::Halt() {
  if( Halted )
    return false;
  Halted     = true;
  SingleStep = false;
  return true;
}

bool RevCore::Resume() {
  if( Halted ) {
    Halted     = false;
    SingleStep = false;
    return true;
  }
  return false;
}

bool RevCore::SingleStepHart() {
  if( SingleStep )
    return true;
  if( Halted ) {
    Halted     = false;
    SingleStep = true;
    return true;
  } else {
    // must be halted to single step
    return false;
  }
}

void RevCore::SetCoProc( RevCoProc* coproc ) {
  if( coProc == nullptr ) {
    coProc = coproc;
  } else {
    output->fatal(
      CALL_INFO,
      -1,
      "CONFIG ERROR: Core %u : Attempting to assign a "
      "co-processor when one is already present\n",
      id
    );
  }
}

bool RevCore::EnableExt( RevExt* Ext, bool Opt ) {
  if( !Ext )
    output->fatal( CALL_INFO, -1, "Error: failed to initialize RISC-V extensions\n" );

  output->verbose( CALL_INFO, 6, 0, "Core %" PRIu32 " ; Enabling extension=%s\n", id, Ext->GetName().data() );

  // add the extension to our vector of enabled objects
  Extensions.push_back( std::unique_ptr<RevExt>( Ext ) );

  // retrieve all the target instructions
  const std::vector<RevInstEntry>& IT = Ext->GetInstTable();

  // setup the mapping of InstTable to Ext objects
  InstTable.reserve( InstTable.size() + IT.size() );

  for( unsigned i = 0; i < IT.size(); i++ ) {
    InstTable.push_back( IT[i] );
    auto ExtObj = std::pair<unsigned, unsigned>( Extensions.size() - 1, i );
    EntryToExt.insert( std::pair<unsigned, std::pair<unsigned, unsigned>>( InstTable.size() - 1, ExtObj ) );
  }

  // load the compressed instructions
  if( feature->IsModeEnabled( RV_C ) ) {
    output->verbose( CALL_INFO, 6, 0, "Core %" PRIu32 " ; Enabling compressed extension=%s\n", id, Ext->GetName().data() );

    const std::vector<RevInstEntry>& CT = Ext->GetCInstTable();
    InstTable.reserve( InstTable.size() + CT.size() );

    for( unsigned i = 0; i < CT.size(); i++ ) {
      InstTable.push_back( CT[i] );
      std::pair<unsigned, unsigned> ExtObj = std::pair<unsigned, unsigned>( Extensions.size() - 1, i );
      EntryToExt.insert( std::pair<unsigned, std::pair<unsigned, unsigned>>( InstTable.size() - 1, ExtObj ) );
    }
    // load the optional compressed instructions
    if( Opt ) {
      output->verbose(
        CALL_INFO, 6, 0, "Core %" PRIu32 " ; Enabling optional compressed extension=%s\n", id, Ext->GetName().data()
      );

      const std::vector<RevInstEntry>& OT = Ext->GetOInstTable();
      InstTable.reserve( InstTable.size() + OT.size() );

      for( unsigned i = 0; i < OT.size(); i++ ) {
        InstTable.push_back( OT[i] );
        std::pair<unsigned, unsigned> ExtObj = std::pair<unsigned, unsigned>( Extensions.size() - 1, i );
        EntryToExt.insert( std::pair<unsigned, std::pair<unsigned, unsigned>>( InstTable.size() - 1, ExtObj ) );
      }
    }
  }

  return true;
}

bool RevCore::SeedInstTable() {
  output->verbose(
    CALL_INFO, 6, 0, "Core %" PRIu32 " ; Seeding instruction table for machine model=%s\n", id, feature->GetMachineModel().data()
  );

  // I-Extension
  if( feature->IsModeEnabled( RV_I ) ) {
    if( feature->IsRV64() ) {
      // load RV32I & RV64; no optional compressed
      EnableExt( new RV32I( feature, mem, output ), false );
      EnableExt( new RV64I( feature, mem, output ), false );
    } else {
      // load RV32I w/ optional compressed
      EnableExt( new RV32I( feature, mem, output ), true );
    }
  }

  // M-Extension
  if( feature->IsModeEnabled( RV_M ) ) {
    EnableExt( new RV32M( feature, mem, output ), false );
    if( feature->IsRV64() ) {
      EnableExt( new RV64M( feature, mem, output ), false );
    }
  }

  // A-Extension
  if( feature->IsModeEnabled( RV_A ) ) {
    EnableExt( new RV32A( feature, mem, output ), false );
    if( feature->IsRV64() ) {
      EnableExt( new RV64A( feature, mem, output ), false );
    }
  }

  // F-Extension
  if( feature->IsModeEnabled( RV_F ) ) {
    if( !feature->IsModeEnabled( RV_D ) && feature->IsRV32() ) {
      EnableExt( new RV32F( feature, mem, output ), true );
    } else {
      EnableExt( new RV32F( feature, mem, output ), false );
      EnableExt( new RV64F( feature, mem, output ), false );
    }
  }

  // D-Extension
  if( feature->IsModeEnabled( RV_D ) ) {
    EnableExt( new RV32D( feature, mem, output ), false );
    if( feature->IsRV64() ) {
      EnableExt( new RV64D( feature, mem, output ), false );
    }
  }

  // Zicbom-Extension
  if( feature->IsModeEnabled( RV_ZICBOM ) ) {
    EnableExt( new Zicbom( feature, mem, output ), false );
  }

  // Zifencei-Extension
  if( feature->IsModeEnabled( RV_ZIFENCEI ) ) {
    EnableExt( new Zifencei( feature, mem, output ), false );
  }

  return true;
}

uint32_t RevCore::CompressCEncoding( RevInstEntry Entry ) {
  uint32_t Value = 0x00;

  Value |= Entry.opcode;
  Value |= uint32_t( Entry.funct2 ) << 2;
  Value |= uint32_t( Entry.funct3 ) << 4;
  Value |= uint32_t( Entry.funct4 ) << 8;
  Value |= uint32_t( Entry.funct6 ) << 12;

  return Value;
}

uint32_t RevCore::CompressEncoding( RevInstEntry Entry ) {
  uint32_t Value = 0x00;

  Value |= Entry.opcode;
  Value |= uint32_t( Entry.funct3 ) << 8;
  Value |= uint32_t( Entry.funct2or7 ) << 11;
  Value |= uint32_t( Entry.imm12 ) << 18;
  // this is a 5 bit field, but only the lower two bits are used, so it *just*
  // fits without going to a uint64
  Value |= uint32_t( Entry.fpcvtOp ) << 30;

  return Value;
}

void RevCore::splitStr( const std::string& s, char c, std::vector<std::string>& v ) {
  std::string::size_type i = 0;
  std::string::size_type j = s.find( c );

  // catch strings with no delims
  if( j == std::string::npos ) {
    v.push_back( s );
  }

  // break up the rest of the string
  while( j != std::string::npos ) {
    v.push_back( s.substr( i, j - i ) );
    i = ++j;
    j = s.find( c, j );
    if( j == std::string::npos )
      v.push_back( s.substr( i, s.length() ) );
  }
}

std::string RevCore::ExtractMnemonic( RevInstEntry Entry ) {
  std::string              Tmp = Entry.mnemonic;
  std::vector<std::string> vstr;
  splitStr( Tmp, ' ', vstr );

  return vstr[0];
}

bool RevCore::InitTableMapping() {
  output->verbose(
    CALL_INFO, 6, 0, "Core %" PRIu32 " ; Initializing table mapping for machine model=%s\n", id, feature->GetMachineModel().data()
  );

  for( unsigned i = 0; i < InstTable.size(); i++ ) {
    NameToEntry.insert( std::pair<std::string, unsigned>( ExtractMnemonic( InstTable[i] ), i ) );
    if( !InstTable[i].compressed ) {
      // map normal instruction
      EncToEntry.insert( std::pair<uint32_t, unsigned>( CompressEncoding( InstTable[i] ), i ) );
      output->verbose(
        CALL_INFO,
        6,
        0,
        "Core %" PRIu32 " ; Table Entry %" PRIu32 " = %s\n",
        id,
        CompressEncoding( InstTable[i] ),
        ExtractMnemonic( InstTable[i] ).data()
      );
    } else {
      // map compressed instruction
      CEncToEntry.insert( std::pair<uint32_t, unsigned>( CompressCEncoding( InstTable[i] ), i ) );
      output->verbose(
        CALL_INFO,
        6,
        0,
        "Core %" PRIu32 " ; Compressed Table Entry %" PRIu32 " = %s\n",
        id,
        CompressCEncoding( InstTable[i] ),
        ExtractMnemonic( InstTable[i] ).data()
      );
    }
  }
  return true;
}

bool RevCore::ReadOverrideTables() {
  output->verbose(
    CALL_INFO, 6, 0, "Core %" PRIu32 " ; Reading override tables for machine model=%s\n", id, feature->GetMachineModel().data()
  );

  std::string Table;
  if( !opts->GetInstTable( id, Table ) )
    return false;

  // if the length of the file name is 0, just return
  if( Table == "_REV_INTERNAL_" )
    return true;

  // open the file
  std::ifstream infile( Table );
  if( !infile.is_open() )
    output->fatal( CALL_INFO, -1, "Error: failed to read instruction table for core=%" PRIu32 "\n", id );

  // read all the values
  std::string Inst;
  std::string Cost;
  unsigned    Entry;
  while( infile >> Inst >> Cost ) {
    auto it = NameToEntry.find( Inst );
    if( it == NameToEntry.end() )
      output->fatal( CALL_INFO, -1, "Error: could not find instruction in table for map value=%s\n", Inst.data() );

    Entry                 = it->second;
    InstTable[Entry].cost = (unsigned) ( std::stoi( Cost, nullptr, 0 ) );
  }

  // close the file
  infile.close();

  return true;
}

bool RevCore::LoadInstructionTable() {
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

bool RevCore::Reset() {

  IdleHarts.reset();

  // All harts are idle to start
  for( unsigned i = 0; i < numHarts; i++ ) {
    IdleHarts[i]  = true;
    ValidHarts[i] = true;
  }
  // Other state bitsets are the unset

  Pipeline.clear();
  CoProcStallReq.reset();

  return true;
}

RevInst RevCore::DecodeCRInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct4 = InstTable[Entry].funct4;

  // registers
  CompInst.rd = CompInst.rs1 = DECODE_RD( Inst );
  CompInst.rs2               = DECODE_LOWER_CRS2( Inst );
  CompInst.imm               = 0x00;

  if( 0b10 == CompInst.opcode ) {
    if( 0b1000 == CompInst.funct4 ) {
      if( 0 != CompInst.rs2 ) {
        // if c.mv force rs1 to x0
        CompInst.rs1 = 0;
      } else {
        // if c.jr force rd to x0
        CompInst.rd = 0;
      }
    } else if( 0b1001 == CompInst.funct4 ) {
      if( 0 != CompInst.rs2 ) {
        // if c.add
      } else if( 0 != CompInst.rs1 ) {
        // if c.jalr force rd to x1
        CompInst.rd = 1;
      } else {
        // if c.ebreak
      }
    }
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCIInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rd = CompInst.rs1 = DECODE_RD( Inst );
  CompInst.imm               = DECODE_LOWER_CRS2( Inst );
  CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );

  if( ( CompInst.opcode == 0b10 ) && ( CompInst.funct3 == 0b001 ) ) {
    // c.fldsp
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1100000 ) >> 2 );         // [4:3]
    CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );  // [5]
    CompInst.imm |= ( ( Inst & 0b11100 ) << 4 );          // [8:6]
    CompInst.rs1 = 2;                                     // Force rs1 to be x2 (stack pointer)
  } else if( ( CompInst.opcode == 0b10 ) && ( CompInst.funct3 == 0b010 ) ) {
    // c.lwsp
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1110000 ) >> 2 );         // [4:2]
    CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );  // [5]
    CompInst.imm |= ( ( Inst & 1100 ) << 4 );             // [7:6]
    CompInst.rs1 = 2;                                     // Force rs1 to be x2 (stack pointer)
  } else if( ( CompInst.opcode == 0b10 ) && ( CompInst.funct3 == 0b011 ) ) {
    CompInst.imm = 0;
    if( feature->IsRV64() ) {
      // c.ldsp
      CompInst.imm = ( ( Inst & 0b1100000 ) >> 2 );         // [4:3]
      CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );  // [5]
      CompInst.imm |= ( ( Inst & 0b11100 ) << 4 );          // [8:6]
      CompInst.rs1 = 2;                                     // Force rs1 to be x2 (stack pointer)
    } else {
      // c.flwsp
      CompInst.imm = ( ( Inst & 0b1110000 ) >> 2 );         // [4:2]
      CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );  // [5]
      CompInst.imm |= ( ( Inst & 1100 ) << 4 );             // [7:6]
      CompInst.rs1 = 2;                                     // Force rs1 to be x2 (stack pointer)
    }
  } else if( ( CompInst.opcode == 0b01 ) && ( CompInst.funct3 == 0b011 ) && ( CompInst.rd == 2 ) ) {
    // c.addi16sp
    // swizzle: nzimm[4|6|8:7|5] nzimm[9]
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1000000 ) >> 2 );         // bit 4
    CompInst.imm |= ( ( Inst & 0b100 ) << 3 );            // bit 5
    CompInst.imm |= ( ( Inst & 0b100000 ) << 1 );         // bit 6
    CompInst.imm |= ( ( Inst & 0b11000 ) << 4 );          // bit 8:7
    CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 3 );  // bit 9
    CompInst.rs1 = 2;                                     // Force rs1 to be x2 (stack pointer)
    // sign extend
    CompInst.imm = CompInst.ImmSignExt( 10 );
  } else if( ( CompInst.opcode == 0b01 ) && ( CompInst.funct3 == 0b011 ) && ( CompInst.rd != 0 ) && ( CompInst.rd != 2 ) ) {
    // c.lui
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1111100 ) << 10 );        // [16:12]
    CompInst.imm |= ( ( Inst & 0b1000000000000 ) << 5 );  // [17]
    // sign extend
    CompInst.imm = CompInst.ImmSignExt( 18 );
    CompInst.imm >>= 12;  //immd value will be re-aligned on execution
  } else if( ( CompInst.opcode == 0b01 ) && ( CompInst.funct3 == 0b010 ) && ( CompInst.rd != 0 ) ) {
    // c.li
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1111100 ) >> 2 );         // [4:0]
    CompInst.imm |= ( ( Inst & 0b1000000000000 ) >> 7 );  // [5]
    CompInst.rs1 = 0;                                     // Force rs1 to be x0, expands to add rd, x0, imm
    // sign extend
    CompInst.imm = CompInst.ImmSignExt( 6 );
  } else {
    // sign extend
    CompInst.imm = CompInst.ImmSignExt( 6 );
  }

  //if c.addi, expands to addi %rd, %rd, $imm so set rs1 to rd -or-
  // c.slli, expands to slli %rd %rd $imm -or -
  // c.addiw. expands to addiw %rd %rd $imm
  if( ( ( 0b01 == CompInst.opcode ) && ( 0b000 == CompInst.funct3 ) ) ||
      ( ( 0b10 == CompInst.opcode ) && ( 0b000 == CompInst.funct3 ) ) ||
      ( ( 0b01 == CompInst.opcode ) && ( 0b001 == CompInst.funct3 ) ) ) {
    CompInst.rs1 = CompInst.rd;
  }
  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCSSInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rs2    = DECODE_LOWER_CRS2( Inst );
  CompInst.imm    = ( ( Inst & 0b1111110000000 ) >> 7 );

  if( CompInst.funct3 == 0b101 ) {
    // c.fsdsp
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
    CompInst.imm |= ( ( Inst & 0b1110000000 ) >> 1 );    // [8:6]
    CompInst.rs1 = 2;                                    // Force rs1 to x2 (stack pointer)
  } else if( CompInst.funct3 == 0b110 ) {
    // c.swsp
    CompInst.imm = 0;
    CompInst.imm = ( ( Inst & 0b1111000000000 ) >> 7 );  // [5:2]
    CompInst.imm |= ( ( Inst & 0b110000000 ) >> 1 );     // [7:6]
    CompInst.rs1 = 2;                                    // Force rs1 to x2 (stack pointer)
  } else if( CompInst.funct3 == 0b111 ) {
    CompInst.imm = 0;
    if( feature->IsRV64() ) {
      // c.sdsp
      CompInst.imm = ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
      CompInst.imm |= ( ( Inst & 0b1110000000 ) >> 1 );    // [8:6]
      CompInst.rs1 = 2;                                    // Force rs1 to x2 (stack pointer)
    } else {
      // c.fswsp
      CompInst.imm = ( ( Inst & 0b1111000000000 ) >> 7 );  // [5:2]
      CompInst.imm |= ( ( Inst & 0b110000000 ) >> 1 );     // [7:6]
      CompInst.rs1 = 2;                                    // Force rs1 to x2 (stack pointer)
    }
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCIWInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rd     = ( ( Inst & 0b11100 ) >> 2 );
  CompInst.imm    = ( ( Inst & 0b1111111100000 ) >> 5 );

  // Apply compressed offset
  CompInst.rd     = CRegIdx( CompInst.rd );

  //swizzle: nzuimm[5:4|9:6|2|3]
  std::bitset<32> imm( CompInst.imm ), tmp;
  tmp[0]       = imm[1];
  tmp[1]       = imm[0];
  tmp[2]       = imm[6];
  tmp[3]       = imm[7];
  tmp[4]       = imm[2];
  tmp[5]       = imm[3];
  tmp[6]       = imm[4];
  tmp[7]       = imm[5];

  CompInst.imm = tmp.to_ulong();

  // Set rs1 to x2 and scale offset by 4 if this is an addi4spn
  if( ( 0x00 == CompInst.opcode ) && ( 0x00 == CompInst.funct3 ) ) {
    CompInst.imm = ( CompInst.imm & 0b11111111 ) * 4;
    CompInst.rs1 = 2;
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCLInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rd     = ( ( Inst & 0b11100 ) >> 2 );
  CompInst.rs1    = ( ( Inst & 0b1110000000 ) >> 7 );

  //Apply compressed offset
  CompInst.rd     = CRegIdx( CompInst.rd );
  CompInst.rs1    = CRegIdx( CompInst.rs1 );

  if( CompInst.funct3 == 0b001 ) {
    // c.fld
    CompInst.imm = ( ( Inst & 0b1100000 ) << 1 );         // [7:6]
    CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
  } else if( CompInst.funct3 == 0b010 ) {
    // c.lw
    CompInst.imm = ( ( Inst & 0b100000 ) << 1 );          // [6]
    CompInst.imm |= ( ( Inst & 0b1000000 ) >> 4 );        // [2]
    CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
  } else if( CompInst.funct3 == 0b011 ) {
    if( feature->IsRV64() ) {
      // c.ld
      CompInst.imm = ( ( Inst & 0b1100000 ) << 1 );         // [7:6]
      CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
    } else {
      // c.flw
      CompInst.imm = ( ( Inst & 0b100000 ) << 1 );          // [6]
      CompInst.imm |= ( ( Inst & 0b1000000 ) >> 4 );        // [2]
      CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
    }
  } else if( CompInst.funct3 == 0b101 ) {
    // c.fsd
    CompInst.imm = ( ( Inst & 0b1100000 ) << 1 );         // [7:6]
    CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
  } else if( CompInst.funct3 == 0b110 ) {
    // c.sw
    CompInst.imm = ( ( Inst & 0b100000 ) << 1 );          // [6]
    CompInst.imm |= ( ( Inst & 0b1000000 ) >> 4 );        // [2]
    CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
  } else if( CompInst.funct3 == 0b111 ) {
    if( feature->IsRV64() ) {
      // c.sd
      CompInst.imm = ( ( Inst & 0b1100000 ) << 1 );         // [7:6]
      CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
    } else {
      // c.fsw
      CompInst.imm = ( ( Inst & 0b100000 ) << 1 );          // [6]
      CompInst.imm |= ( ( Inst & 0b1000000 ) >> 4 );        // [2]
      CompInst.imm |= ( ( Inst & 0b1110000000000 ) >> 7 );  // [5:3]
    }
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCSInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rs2    = ( ( Inst & 0b011100 ) >> 2 );
  CompInst.rs1    = ( ( Inst & 0b01110000000 ) >> 7 );

  //Apply Compressed offset
  CompInst.rs2    = CRegIdx( CompInst.rs2 );
  CompInst.rs1    = CRegIdx( CompInst.rs1 );

  // The immd is pre-scaled in this instruction format
  if( CompInst.funct3 == 0b110 ) {
    //c.sw
    CompInst.imm = ( ( Inst & 0b0100000 ) << 1 );          //offset[6]
    CompInst.imm |= ( ( Inst & 0b01110000000000 ) >> 6 );  //offset[5:3]
    CompInst.imm |= ( ( Inst & 0b01000000 ) >> 4 );        //offset[2]
  } else {
    if( feature->IsRV32() ) {
      //c.fsw
      CompInst.imm = ( ( Inst & 0b00100000 ) << 1 );         //imm[6]
      CompInst.imm = ( ( Inst & 0b01000000 ) << 4 );         //imm[2]
      CompInst.imm |= ( ( Inst & 0b01110000000000 ) >> 7 );  //imm[5:3]
    } else {
      //c.sd
      CompInst.imm = ( ( Inst & 0b01100000 ) << 1 );         //imm[7:6]
      CompInst.imm |= ( ( Inst & 0b01110000000000 ) >> 7 );  //imm[5:3]
    }
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCAInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct2 = InstTable[Entry].funct2;
  CompInst.funct6 = InstTable[Entry].funct6;

  // registers
  CompInst.rs2    = ( ( Inst & 0b11100 ) >> 2 );
  CompInst.rd = CompInst.rs1 = ( ( Inst & 0b1110000000 ) >> 7 );

  //Adjust registers for compressed offset
  CompInst.rs2               = CRegIdx( CompInst.rs2 );
  CompInst.rs1               = CRegIdx( CompInst.rs1 );
  CompInst.rd                = CRegIdx( CompInst.rd );

  //All instructions of this format expand to <opcode> rd rd rs2, so set rs1 to rd
  CompInst.rs1               = CompInst.rd;

  CompInst.instSize          = 2;
  CompInst.compressed        = true;

  return CompInst;
}

RevInst RevCore::DecodeCBInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  CompInst.rd = CompInst.rs1 = ( ( Inst & 0b1110000000 ) >> 7 );
  CompInst.offset            = ( ( Inst & 0b1111100 ) >> 2 );
  CompInst.offset |= ( ( Inst & 0b1110000000000 ) >> 5 );

  //Apply compressed offset
  CompInst.rs1 = CRegIdx( CompInst.rs1 );

  //If c.srli, c.srai or c.andi set rd to rs1
  if( ( 0b01 == CompInst.opcode ) && ( 0b100 == CompInst.funct3 ) ) {
    CompInst.rd = CompInst.rs1;
  }

  //swizzle: offset[8|4:3]  offset[7:6|2:1|5]
  std::bitset<16> tmp;
  // handle c.beqz/c.bnez offset
  if( ( CompInst.opcode == 0b01 ) && ( CompInst.funct3 >= 0b110 ) ) {
    std::bitset<16> o( CompInst.offset );
    tmp[0] = o[1];
    tmp[1] = o[2];
    tmp[2] = o[5];
    tmp[3] = o[6];
    tmp[4] = o[0];
    tmp[5] = o[3];
    tmp[6] = o[4];
    tmp[7] = o[7];
  }

  CompInst.offset = ( (uint16_t) tmp.to_ulong() ) << 1;  // scale to corrrect position to be consistent with other compressed ops

  if( ( 0b01 == CompInst.opcode ) && ( CompInst.funct3 >= 0b110 ) ) {
    //Set rs2 to x0 if c.beqz or c.bnez
    CompInst.rs2 = 0;
    CompInst.imm = CompInst.offset;
    CompInst.imm = CompInst.ImmSignExt( 9 );
  } else {
    CompInst.imm = ( ( Inst & 0b01111100 ) >> 2 );
    CompInst.imm |= ( ( Inst & 0b01000000000000 ) >> 7 );
    CompInst.imm = CompInst.ImmSignExt( 6 );
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

RevInst RevCore::DecodeCJInst( uint16_t Inst, unsigned Entry ) const {
  RevInst CompInst;

  // cost
  CompInst.cost   = InstTable[Entry].cost;

  // encodings
  CompInst.opcode = InstTable[Entry].opcode;
  CompInst.funct3 = InstTable[Entry].funct3;

  // registers
  uint16_t offset = ( ( Inst & 0b1111111111100 ) >> 2 );

  //swizzle bits offset[11|4|9:8|10|6|7|3:1|5]
  std::bitset<16> offsetBits( offset ), target;
  target[0]           = offsetBits[1];
  target[1]           = offsetBits[2];
  target[2]           = offsetBits[3];
  target[3]           = offsetBits[9];
  target[4]           = offsetBits[0];
  target[5]           = offsetBits[5];
  target[6]           = offsetBits[4];
  target[7]           = offsetBits[7];
  target[8]           = offsetBits[8];
  target[9]           = offsetBits[6];
  target[10]          = offsetBits[10];
  CompInst.jumpTarget = ( (u_int16_t) target.to_ulong() ) << 1;

  if( ( 0b01 == CompInst.opcode ) && ( 0b001 == CompInst.funct3 || 0b101 == CompInst.funct3 ) ) {
    //Set rd to x1 if this is a c.jal, x0 if this is a c.j
    CompInst.rd  = ( 0b001 == CompInst.funct3 ) ? 1 : 0;
    CompInst.imm = CompInst.jumpTarget;
    CompInst.imm = CompInst.ImmSignExt( 12 );
  }

  CompInst.instSize   = 2;
  CompInst.compressed = true;

  return CompInst;
}

// Find the first matching encoding which satisfies a predicate, if any
auto RevCore::matchInst(
  const std::unordered_multimap<uint32_t, unsigned>& map,
  uint32_t                                           encoding,
  const std::vector<RevInstEntry>&                   InstTable,
  uint32_t                                           Inst
) const {
  // Iterate through all entries which match the encoding
  for( auto [it, end] = map.equal_range( encoding ); it != end; ++it ) {
    unsigned Entry = it->second;
    // If an entry is valid and has a satisfied predicate, return it
    if( Entry < InstTable.size() && InstTable[Entry].predicate( Inst ) )
      return it;
  }

  // No match
  return map.end();
}

RevInst RevCore::DecodeCompressed( uint32_t Inst ) const {
  uint8_t  opc    = 0;
  uint8_t  funct2 = 0;
  uint8_t  funct3 = 0;
  uint8_t  funct4 = 0;
  uint8_t  funct6 = 0;
  uint8_t  l3     = 0;
  uint32_t Enc    = 0x00ul;

  if( !feature->HasCompressed() ) {
    output->fatal(
      CALL_INFO, -1, "Error: failed to decode instruction at PC=0x%" PRIx64 "; Compressed instructions not enabled!\n", GetPC()
    );
  }

  // Truncate instruction to the first 16 bits
  Inst = static_cast<uint16_t>( Inst );

  // decode the opcode
  opc  = ( Inst & 0b11 );
  l3   = ( ( Inst & 0b1110000000000000 ) >> 13 );
  if( opc == 0b00 ) {
    // quadrant 0
    funct3 = l3;
  } else if( opc == 0b01 ) {
    // quadrant 1
    if( l3 <= 0b011 ) {
      // upper portion: misc
      funct3 = l3;
    } else if( ( l3 > 0b011 ) && ( l3 < 0b101 ) ) {
      // middle portion: arithmetics
      uint8_t opSelect = ( ( Inst & 0b110000000000 ) >> 10 );
      if( opSelect == 0b11 ) {
        funct6 = ( ( Inst & 0b1111110000000000 ) >> 10 );
        funct2 = ( ( Inst & 0b01100000 ) >> 5 );
      } else {
        funct3 = l3;
        funct2 = opSelect;
      }
    } else {
      // lower power: jumps/branches
      funct3 = l3;
    }
  } else if( opc == 0b10 ) {
    // quadrant 2
    if( l3 == 0b000 ) {
      // slli{64}
      funct3 = l3;
    } else if( l3 < 0b100 ) {
      // float/double/quad load
      funct3 = l3;
    } else if( l3 == 0b100 ) {
      // jump, mv, break, add
      funct4 = ( ( Inst & 0b1111000000000000 ) >> 12 );
    } else {
      // float/double/quad store
      funct3 = l3;
    }
  }

  Enc |= (uint32_t) ( opc );
  Enc |= (uint32_t) ( funct2 << 2 );
  Enc |= (uint32_t) ( funct3 << 4 );
  Enc |= (uint32_t) ( funct4 << 8 );
  Enc |= (uint32_t) ( funct6 << 12 );

  bool isCoProcInst = false;
  auto it           = matchInst( CEncToEntry, Enc, InstTable, Inst );
  if( it == CEncToEntry.end() ) {
    if( coProc && coProc->IssueInst( feature, RegFile, mem, Inst ) ) {
      isCoProcInst     = true;
      //Create NOP - ADDI x0, x0, 0
      uint8_t caddi_op = 0b01;
      Inst             = 0;
      Enc              = 0;
      Enc |= caddi_op;
      it = matchInst( CEncToEntry, Enc, InstTable, Inst );
    }
  }

  if( it == CEncToEntry.end() ) {
    output->fatal(
      CALL_INFO,
      -1,
      "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%" PRIu32
      "\n opc=%x; funct2=%x, funct3=%x, funct4=%x, funct6=%x\n",
      GetPC(),
      Enc,
      opc,
      funct2,
      funct3,
      funct4,
      funct6
    );
  }

  auto Entry = it->second;
  if( Entry >= InstTable.size() ) {
    output->fatal(
      CALL_INFO,
      -1,
      "Error: no entry in table for instruction at PC=0x%" PRIx64 " Opcode = %x Funct2 = %x Funct3 = %x Funct4 = %x Funct6 = "
      "%x Enc = %x \n",
      GetPC(),
      opc,
      funct2,
      funct3,
      funct4,
      funct6,
      Enc
    );
  }

  RevInst ret{};

  switch( InstTable[Entry].format ) {
  case RVCTypeCR: ret = DecodeCRInst( Inst, Entry ); break;
  case RVCTypeCI: ret = DecodeCIInst( Inst, Entry ); break;
  case RVCTypeCSS: ret = DecodeCSSInst( Inst, Entry ); break;
  case RVCTypeCIW: ret = DecodeCIWInst( Inst, Entry ); break;
  case RVCTypeCL: ret = DecodeCLInst( Inst, Entry ); break;
  case RVCTypeCS: ret = DecodeCSInst( Inst, Entry ); break;
  case RVCTypeCA: ret = DecodeCAInst( Inst, Entry ); break;
  case RVCTypeCB: ret = DecodeCBInst( Inst, Entry ); break;
  case RVCTypeCJ: ret = DecodeCJInst( Inst, Entry ); break;
  default: output->fatal( CALL_INFO, -1, "Error: failed to decode instruction format at PC=%" PRIx64 ".", GetPC() );
  }

  ret.entry        = Entry;
  ret.isCoProcInst = isCoProcInst;
  return ret;
}

RevInst RevCore::DecodeRInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = InstTable[Entry].funct3;
  DInst.funct2or7 = InstTable[Entry].funct2or7;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rdClass != RevRegClass::RegUNKNOWN ) {
    DInst.rd = DECODE_RD( Inst );
  }
  if( InstTable[Entry].rs1Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs1 = DECODE_RS1( Inst );
  }
  if( InstTable[Entry].rs2Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs2 = DECODE_RS2( Inst );
  }

  // imm
  if( ( InstTable[Entry].imm == FImm ) && ( InstTable[Entry].rs2Class == RevRegClass::RegUNKNOWN ) ) {
    DInst.imm = DECODE_IMM12( Inst ) & 0b011111;
  } else {
    DInst.imm = 0x0;
  }

  // Size
  DInst.instSize = 4;

  // Decode the atomic RL/AQ fields
  if( DInst.opcode == 0b0101111 ) {
    DInst.rl = DECODE_RL( Inst );
    DInst.aq = DECODE_AQ( Inst );
  }

  // Decode any ancillary SP/DP float options
  if( DInst.opcode == 0b1010011 ) {
    DInst.rm = DECODE_RM( Inst );
  }

  DInst.compressed = false;

  return DInst;
}

RevInst RevCore::DecodeIInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = InstTable[Entry].funct3;
  DInst.funct2or7 = 0x0;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rdClass != RevRegClass::RegUNKNOWN ) {
    DInst.rd = DECODE_RD( Inst );
  }
  if( InstTable[Entry].rs1Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs1 = DECODE_RS1( Inst );
  }

  // imm
  DInst.imm        = DECODE_IMM12( Inst );

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;

  return DInst;
}

RevInst RevCore::DecodeSInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = InstTable[Entry].funct3;
  DInst.funct2or7 = 0x0;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rs1Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs1 = DECODE_RS1( Inst );
  }
  if( InstTable[Entry].rs2Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs2 = DECODE_RS2( Inst );
  }

  // imm
  DInst.imm        = ( DECODE_RD( Inst ) | ( DECODE_FUNCT7( Inst ) << 5 ) );

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevCore::DecodeUInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = 0x0;
  DInst.funct2or7 = 0x0;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rdClass != RevRegClass::RegUNKNOWN ) {
    DInst.rd = DECODE_RD( Inst );
  }

  // imm
  DInst.imm        = DECODE_IMM20( Inst );

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevCore::DecodeBInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = InstTable[Entry].funct3;
  DInst.funct2or7 = 0x0;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rs1Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs1 = DECODE_RS1( Inst );
  }
  if( InstTable[Entry].rs2Class != RevRegClass::RegUNKNOWN ) {
    DInst.rs2 = DECODE_RS2( Inst );
  }

  // imm
  DInst.imm = ( ( Inst >> 19 ) & 0b1000000000000 ) |  // [12]
              ( ( Inst << 4 ) & 0b100000000000 ) |    // [11]
              ( ( Inst >> 20 ) & 0b11111100000 ) |    // [10:5]
              ( ( Inst >> 7 ) & 0b11110 );            // [4:1]

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevCore::DecodeJInst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost      = InstTable[Entry].cost;

  // encodings
  DInst.opcode    = InstTable[Entry].opcode;
  DInst.funct3    = InstTable[Entry].funct3;
  DInst.funct2or7 = 0x0;

  // registers
  DInst.rd        = 0x0;
  DInst.rs1       = 0x0;
  DInst.rs2       = 0x0;
  DInst.rs3       = 0x0;

  if( InstTable[Entry].rdClass != RevRegClass::RegUNKNOWN ) {
    DInst.rd = DECODE_RD( Inst );
  }

  // immA
  DInst.imm = ( ( Inst >> 11 ) & 0b100000000000000000000 ) |  // imm[20]
              ( (Inst) &0b11111111000000000000 ) |            // imm[19:12]
              ( ( Inst >> 9 ) & 0b100000000000 ) |            // imm[11]
              ( ( Inst >> 20 ) & 0b11111111110 );             // imm[10:1]

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;
  return DInst;
}

RevInst RevCore::DecodeR4Inst( uint32_t Inst, unsigned Entry ) const {
  RevInst DInst;

  // cost
  DInst.cost       = InstTable[Entry].cost;

  // encodings
  DInst.opcode     = InstTable[Entry].opcode;
  DInst.funct3     = 0x0;
  DInst.funct2or7  = DECODE_FUNCT2( Inst );
  DInst.rm         = DECODE_RM( Inst );

  // registers
  DInst.rd         = DECODE_RD( Inst );
  DInst.rs1        = DECODE_RS1( Inst );
  DInst.rs2        = DECODE_RS2( Inst );
  DInst.rs3        = DECODE_RS3( Inst );

  // imm
  DInst.imm        = 0x0;

  // Size
  DInst.instSize   = 4;

  DInst.compressed = false;
  return DInst;
}

bool RevCore::DebugReadReg( unsigned Idx, uint64_t* Value ) const {
  if( !Halted )
    return false;
  if( Idx >= _REV_NUM_REGS_ ) {
    return false;
  }
  RevRegFile* regFile = GetRegFile( HartToExecID );
  *Value              = regFile->GetX<uint64_t>( Idx );
  return true;
}

bool RevCore::DebugWriteReg( unsigned Idx, uint64_t Value ) const {
  RevRegFile* regFile = GetRegFile( HartToExecID );
  if( !Halted )
    return false;
  if( Idx >= _REV_NUM_REGS_ ) {
    return false;
  }
  regFile->SetX( Idx, Value );
  return true;
}

bool RevCore::PrefetchInst() {
  uint64_t PC = Harts[HartToDecodeID]->RegFile->GetPC();

  // These are addresses that we can't decode
  // Return false back to the main program loop
  if( PC == 0x00ull ) {
    return false;
  }

  return sfetch->IsAvail( PC );
}

RevInst RevCore::FetchAndDecodeInst() {
  uint32_t Inst    = 0x00ul;
  uint64_t PC      = GetPC();
  bool     Fetched = false;

  // Stage 1: Retrieve the instruction
  if( !sfetch->InstFetch( PC, Fetched, Inst ) ) {
    output->fatal( CALL_INFO, -1, "Error: failed to retrieve prefetched instruction at PC=0x%" PRIx64 "\n", PC );
  }

  if( 0 != Inst ) {
    output->verbose(
      CALL_INFO,
      6,
      0,
      "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32 "; PC:InstPayload = 0x%" PRIx64 ":0x%" PRIx32 "\n",
      id,
      HartToDecodeID,
      ActiveThreadID,
      PC,
      Inst
    );
  } else {
    output->fatal(
      CALL_INFO, -1, "Error: Core %" PRIu32 " failed to decode instruction at PC=0x%" PRIx64 "; Inst=%" PRIu32 "\n", id, PC, Inst
    );
  }

  // Trace capture fetched instruction
  if( Tracer )
    Tracer->SetFetchedInsn( PC, Inst );

  // Stage 1a: handle the crack fault injection
  if( CrackFault ) {
    uint64_t rval = RevRand( 0, ( uint32_t{ 1 } << fault_width ) - 1 );
    Inst |= rval;

    // clear the fault
    CrackFault = false;
  }

  // Decode the instruction
  RevInst DInst = DecodeInst( Inst );

  // Set RegFile Entry and cost, and clear trigger
  RegFile->SetEntry( DInst.entry );
  RegFile->SetCost( DInst.cost );
  RegFile->SetTrigger( false );

  // Return decoded instruction
  return DInst;
}

// Decode the instruction
// This function is pure, with no side effects or dependencies
// on non-constant outside variables. This make it memoizable,
// but right now, there isn't enough benefit for memoization.
RevInst RevCore::DecodeInst( uint32_t Inst ) const {
  if( ~Inst & 0b11 ) {
    // this is a compressed instruction
    return DecodeCompressed( Inst );
  }

  // Stage 2: Retrieve the opcode
  const uint32_t Opcode = Inst & 0b1111111;
  uint32_t       Enc    = 0;

  // Stage 3: Determine if we have a funct3 field
  uint32_t       Funct3 = 0x00ul;
  const uint32_t inst42 = Opcode >> 2 & 0b111;
  const uint32_t inst65 = Opcode >> 5 & 0b11;

  if( ( inst42 == 0b011 ) && ( inst65 == 0b11 ) ) {
    // JAL
    Funct3 = 0x00ul;
  } else if( ( inst42 == 0b101 ) && ( inst65 == 0b00 ) ) {
    // AUIPC
    Funct3 = 0x00ul;
  } else if( ( inst42 == 0b101 ) && ( inst65 == 0b01 ) ) {
    // LUI
    Funct3 = 0x00ul;
  } else {
    // Retrieve the field
    Funct3 = ( ( Inst & 0b111000000000000 ) >> 12 );
  }

  // Stage 4: Determine if we have a funct7 field (R-Type and some specific I-Type)
  uint32_t Funct2or7 = 0x00ul;
  if( inst65 == 0b01 ) {
    if( ( inst42 == 0b011 ) || ( inst42 == 0b100 ) || ( inst42 == 0b110 ) ) {
      // R-Type encodings
      Funct2or7 = ( ( Inst >> 25 ) & 0b1111111 );
      //Atomics have a smaller funct7 field - trim out the aq and rl fields
      if( Opcode == 0b0101111 ) {
        Funct2or7 = ( Funct2or7 & 0b01111100 ) >> 2;
      }
    }
  } else if( ( inst65 == 0b10 ) && ( inst42 < 0b100 ) ) {
    // R4-Type encodings -- we store the Funct2 precision field in Funct2or7
    Funct2or7 = DECODE_FUNCT2( Inst );
  } else if( ( inst65 == 0b10 ) && ( inst42 == 0b100 ) ) {
    // R-Type encodings
    Funct2or7 = ( ( Inst >> 25 ) & 0b1111111 );
  } else if( ( inst65 == 0b00 ) && ( inst42 == 0b110 ) && ( Funct3 != 0 ) ) {
    // R-Type encodings
    Funct2or7 = ( ( Inst >> 25 ) & 0b1111111 );
  } else if( ( inst65 == 0b00 ) && ( inst42 == 0b100 ) && ( Funct3 == 0b101 ) ) {
    // Special I-Type encoding for SRAI - also, Funct7 is only 6 bits in this case
    Funct2or7 = ( ( Inst >> 26 ) & 0b1111111 );
  }

  uint32_t fcvtOp = 0;
  //Special encodings for FCVT instructions
  if( Opcode == 0b1010011 ) {
    switch( Funct2or7 ) {
    case 0b1100000:
    case 0b1101000:
    case 0b0100000:
    case 0b0100001:
    case 0b1100001:
    case 0b1101001: fcvtOp = DECODE_RS2( Inst );
    }
  }

  // Stage 5: Determine if we have an imm12 field (ECALL and EBREAK)
  uint32_t Imm12 = 0x00ul;
  if( ( inst42 == 0b100 ) && ( inst65 == 0b11 ) && ( Funct3 == 0 ) ) {
    Imm12 = DECODE_IMM12( Inst );
  }

  // Stage 6: Compress the encoding
  Enc |= Opcode;
  Enc |= Funct3 << 8;
  Enc |= Funct2or7 << 11;
  Enc |= Imm12 << 18;
  Enc |= fcvtOp << 30;

  // Stage 7: Look up the value in the table
  auto it = matchInst( EncToEntry, Enc, InstTable, Inst );

  // This is kind of a hack, but we may not have found the instruction because
  // Funct3 is overloaded with rounding mode, so if this is a RV32F or RV64F
  // set Funct3 to zero and check again. We exclude if Funct3 == 0b101 ||
  // Funct3 == 0b110 because those are invalid FP rounding mode (rm) values.
  if( inst65 == 0b10 && Funct3 != 0b101 && Funct3 != 0b110 && it == EncToEntry.end() ) {
    Enc &= 0xfffff8ff;
    it = matchInst( EncToEntry, Enc, InstTable, Inst );
  }

  bool isCoProcInst = false;

  // If we did not find a valid instruction, look for a coprocessor instruction
  if( it == EncToEntry.end() && coProc && coProc->IssueInst( feature, RegFile, mem, Inst ) ) {
    isCoProcInst     = true;
    //Create NOP - ADDI x0, x0, 0
    uint32_t addi_op = 0b0010011;
    Inst             = 0;
    Enc              = 0;
    Enc |= addi_op;
    it = matchInst( EncToEntry, Enc, InstTable, Inst );
  }

  if( it == EncToEntry.end() ) {
    // failed to decode the instruction
    output->fatal( CALL_INFO, -1, "Error: failed to decode instruction at PC=0x%" PRIx64 "; Enc=%" PRIu32 "\n", GetPC(), Enc );
  }

  unsigned Entry = it->second;
  if( Entry >= InstTable.size() ) {
    if( coProc && coProc->IssueInst( feature, RegFile, mem, Inst ) ) {
      isCoProcInst     = true;
      //Create NOP - ADDI x0, x0, 0
      uint32_t addi_op = 0b0010011;
      Inst             = 0;
      Enc              = 0;
      Enc |= addi_op;
      it    = matchInst( EncToEntry, Enc, InstTable, Inst );
      Entry = it->second;
    }
  }

  if( Entry >= InstTable.size() ) {
    output->fatal(
      CALL_INFO,
      -1,
      "Error: no entry in table for instruction at PC=0x%" PRIx64 " Opcode = %x Funct3 = %x Funct2or7 = %x Imm12 = %x Enc = %x \n",
      GetPC(),
      Opcode,
      Funct3,
      Funct2or7,
      Imm12,
      Enc
    );
  }

  // Stage 8: Do a full deocode using the target format
  RevInst ret{};
  switch( InstTable[Entry].format ) {
  case RVTypeR: ret = DecodeRInst( Inst, Entry ); break;
  case RVTypeI: ret = DecodeIInst( Inst, Entry ); break;
  case RVTypeS: ret = DecodeSInst( Inst, Entry ); break;
  case RVTypeU: ret = DecodeUInst( Inst, Entry ); break;
  case RVTypeB: ret = DecodeBInst( Inst, Entry ); break;
  case RVTypeJ: ret = DecodeJInst( Inst, Entry ); break;
  case RVTypeR4: ret = DecodeR4Inst( Inst, Entry ); break;
  default: output->fatal( CALL_INFO, -1, "Error: failed to decode instruction format at PC=%" PRIx64 ".", GetPC() );
  }

  ret.entry        = Entry;
  ret.isCoProcInst = isCoProcInst;
  return ret;
}

void RevCore::HandleRegFault( unsigned width ) {
  const char* RegPrefix;
  RevRegFile* regFile = GetRegFile( HartToExecID );

  // select a register
  unsigned RegIdx     = RevRand( 0, _REV_NUM_REGS_ - 1 );

  if( !feature->HasF() || RevRand( 0, 1 ) ) {
    // X registers
    if( feature->IsRV32() ) {
      regFile->RV32[RegIdx] |= RevRand( 0, ~( ~uint32_t{ 0 } << width ) );
    } else {
      regFile->RV64[RegIdx] |= RevRand( 0, ~( ~uint64_t{ 0 } << width ) );
    }
    RegPrefix = "x";
  } else {
    // F registers
    if( feature->HasD() ) {
      uint64_t tmp;
      memcpy( &tmp, &regFile->DPF[RegIdx], sizeof( tmp ) );
      tmp |= RevRand( 0, ~( ~uint32_t{ 0 } << width ) );
      memcpy( &regFile->DPF[RegIdx], &tmp, sizeof( tmp ) );
    } else {
      uint32_t tmp;
      memcpy( &tmp, &regFile->SPF[RegIdx], sizeof( tmp ) );
      tmp |= RevRand( 0, ~( ~uint64_t{ 0 } << width ) );
      memcpy( &regFile->SPF[RegIdx], &tmp, sizeof( tmp ) );
    }
    RegPrefix = "f";
  }

  output->verbose(
    CALL_INFO, 5, 0, "FAULT:REG: Register fault of %" PRIu32 " bits into register %s%" PRIu32 "\n", width, RegPrefix, RegIdx
  );
}

void RevCore::HandleCrackFault( unsigned width ) {
  CrackFault  = true;
  fault_width = width;
  output->verbose( CALL_INFO, 5, 0, "FAULT:CRACK: Crack+Decode fault injected into next decode cycle\n" );
}

void RevCore::HandleALUFault( unsigned width ) {
  ALUFault    = true;
  fault_width = true;
  output->verbose( CALL_INFO, 5, 0, "FAULT:ALU: ALU fault injected into next retire cycle\n" );
}

bool RevCore::DependencyCheck( unsigned HartID, const RevInst* I ) const {
  const RevRegFile*   regFile = GetRegFile( HartID );
  const RevInstEntry* E       = &InstTable[I->entry];

  // For ECALL, check for any outstanding dependencies on a0-a7
  if( I->opcode == 0b1110011 && I->imm == 0 && I->funct3 == 0 && I->rd == 0 && I->rs1 == 0 ) {
    for( RevReg reg : { RevReg::a7, RevReg::a0, RevReg::a1, RevReg::a2, RevReg::a3, RevReg::a4, RevReg::a5, RevReg::a6 } ) {
      if( LSQCheck( HartToDecodeID, RegFile, uint16_t( reg ), RevRegClass::RegGPR ) || ScoreboardCheck( RegFile, uint16_t( reg ), RevRegClass::RegGPR ) ) {
        return true;
      }
    }
    return false;
  }

  return
    // check LS queue for outstanding load
    LSQCheck( HartID, regFile, I->rs1, E->rs1Class ) || LSQCheck( HartID, regFile, I->rs2, E->rs2Class ) ||
    LSQCheck( HartID, regFile, I->rs3, E->rs3Class ) || LSQCheck( HartID, regFile, I->rd, E->rdClass ) ||

    // Iterate through the source registers rs1, rs2, rs3 and find any dependency
    // based on the class of the source register and the associated scoreboard
    ScoreboardCheck( regFile, I->rs1, E->rs1Class ) || ScoreboardCheck( regFile, I->rs2, E->rs2Class ) ||
    ScoreboardCheck( regFile, I->rs3, E->rs3Class );
}

void RevCore::ExternalStallHart( RevCorePasskey<RevCoProc>, uint16_t HartID ) {
  if( HartID < Harts.size() ) {
    CoProcStallReq.set( HartID );
  } else {
    output->fatal( CALL_INFO, -1, "Core %u ; CoProc Request: Cannot stall Hart %" PRIu32 " as the ID is invalid\n", id, HartID );
  }
}

void RevCore::ExternalReleaseHart( RevCorePasskey<RevCoProc>, uint16_t HartID ) {
  if( HartID < Harts.size() ) {
    CoProcStallReq.reset( HartID );
  } else {
    output->fatal( CALL_INFO, -1, "Core %u ; CoProc Request: Cannot release Hart %" PRIu32 " as the ID is invalid\n", id, HartID );
  }
}

unsigned RevCore::GetNextHartToDecodeID() const {
  if( HartsClearToDecode.none() ) {
    return HartToDecodeID;
  };

  unsigned nextID = HartToDecodeID;
  if( HartsClearToDecode[HartToDecodeID] ) {
    nextID = HartToDecodeID;
  } else {
    for( size_t tID = 0; tID < Harts.size(); tID++ ) {
      nextID++;
      if( nextID >= Harts.size() ) {
        nextID = 0;
      }
      if( HartsClearToDecode[nextID] ) {
        break;
      };
    }
    output->verbose(
      CALL_INFO, 6, 0, "Core %" PRIu32 "; Hart switch from %" PRIu32 " to %" PRIu32 "\n", id, HartToDecodeID, nextID
    );
  }
  return nextID;
}

void RevCore::MarkLoadComplete( const MemReq& req ) {
  // Iterate over all outstanding loads for this reg (if any)
  for( auto [i, end] = LSQueue->equal_range( req.LSQHash() ); i != end; ++i ) {
    if( i->second.Addr == req.Addr ) {
      // Only clear the dependency if this is the
      // LAST outstanding load for this register
      if( LSQueue->count( req.LSQHash() ) == 1 ) {
        DependencyClear( req.Hart, req.DestReg, req.RegType );
      }
      sfetch->MarkInstructionLoadComplete( req );
      // Remove this load from the queue
      LSQueue->erase( i );
      return;
    }
  }

  // Instruction prefetch fills target x0; we can ignore these
  if( req.DestReg == 0 && req.RegType == RevRegClass::RegGPR )
    return;
  output->fatal(
    CALL_INFO,
    -1,
    "Core %" PRIu32 "; Hart %" PRIu32 "; "
    "Cannot find matching address for outstanding "
    "load for reg %" PRIu32 " from address %" PRIx64 "\n",
    id,
    req.Hart,
    req.DestReg,
    req.Addr
  );
}

bool RevCore::ClockTick( SST::Cycle_t currentCycle ) {
  RevInst Inst;
  bool    rtn = false;
  Stats.totalCycles++;

  // -- MAIN PROGRAM LOOP --
  //
  // If the clock is down to zero, then fetch the next instruction
  // else if the the instruction has not yet been triggered, execute it
  // else, wait until the counter is decremented to zero to retire the instruction

  // This function updates the bitset of Harts that are
  // ready to decode
  UpdateStatusOfHarts();

  if( HartsClearToDecode.any() && ( !Halted ) ) {
    // Determine what hart is ready to decode
    HartToDecodeID = GetNextHartToDecodeID();
    ActiveThreadID = Harts.at( HartToDecodeID )->GetAssignedThreadID();
    RegFile        = Harts[HartToDecodeID]->RegFile.get();

    feature->SetHartToExecID( HartToDecodeID );

    // fetch the next instruction
    if( !PrefetchInst() ) {
      Stalled = true;
      Stats.cyclesStalled++;
    } else {
      Stalled = false;
    }

    if( !Stalled && !CoProcStallReq[HartToDecodeID] ) {
      Inst       = FetchAndDecodeInst();
      Inst.entry = RegFile->GetEntry();
    }

    // Now that we have decoded the instruction, check for pipeline hazards
    if( ExecEcall() || Stalled || DependencyCheck( HartToDecodeID, &Inst ) || CoProcStallReq[HartToDecodeID] ) {
      RegFile->SetCost( 0 );        // We failed dependency check, so set cost to 0 - this will
      Stats.cyclesIdle_Pipeline++;  // prevent the instruction from advancing to the next stage
      HartsClearToExecute[HartToDecodeID] = false;
      HartToExecID                        = _REV_INVALID_HART_ID_;
    } else {
      Stats.cyclesBusy++;
      HartsClearToExecute[HartToDecodeID] = true;
      HartToExecID                        = HartToDecodeID;
    }
    Inst.cost  = RegFile->GetCost();
    Inst.entry = RegFile->GetEntry();
    rtn        = true;
    ExecPC     = RegFile->GetPC();
  }

  if( ( ( HartToExecID != _REV_INVALID_HART_ID_ ) && !RegFile->GetTrigger() ) && !Halted && HartsClearToExecute[HartToExecID] ) {
    // trigger the next instruction
    // HartToExecID = HartToDecodeID;
    RegFile->SetTrigger( true );

#ifdef NO_REV_TRACER
    // pull the PC
    output->verbose(
      CALL_INFO,
      6,
      0,
      "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32 "; Executing PC= 0x%" PRIx64 "\n",
      id,
      HartToExecID,
      ActiveThreadID,
      ExecPC
    );
#endif

    // Find the instruction extension
    auto it = EntryToExt.find( RegFile->GetEntry() );
    if( it == EntryToExt.end() ) {
      // failed to find the extension
      output->fatal( CALL_INFO, -1, "Error: failed to find the instruction extension at PC=%" PRIx64 ".", ExecPC );
    }

    // found the instruction extension
    std::pair<unsigned, unsigned> EToE = it->second;
    RevExt*                       Ext  = Extensions[EToE.first].get();

    // -- BEGIN new pipelining implementation
    Pipeline.emplace_back( std::make_pair( HartToExecID, Inst ) );

    if( ( Ext->GetName() == "RV32F" ) || ( Ext->GetName() == "RV32D" ) || ( Ext->GetName() == "RV64F" ) || ( Ext->GetName() == "RV64D" ) ) {
      Stats.floatsExec++;
    }

    // set the hazarding
    DependencySet( HartToExecID, &( Pipeline.back().second ) );
    // -- END new pipelining implementation

#ifndef NO_REV_TRACER
    // Tracer context
    mem->SetTracer( Tracer );
    RegFile->SetTracer( Tracer );
#endif

    // execute the instruction
    if( !Ext->Execute( EToE.second, Pipeline.back().second, HartToExecID, RegFile ) ) {
      output->fatal( CALL_INFO, -1, "Error: failed to execute instruction at PC=%" PRIx64 ".", ExecPC );
    }

#ifndef NO_REV_TRACER
    // Clear memory tracer so we don't pick up instruction fetches and other access.
    // TODO: method to determine origin of memory access (core, cache, pan, host debugger, ... )
    mem->SetTracer( nullptr );
    // Conditionally trace after execution
    if( Tracer )
      Tracer->Exec( currentCycle, id, HartToExecID, ActiveThreadID, InstTable[Inst.entry].mnemonic );
#endif

#ifdef __REV_DEEP_TRACE__
    if( feature->IsRV32() ) {
      std::cout << "RDT: Executed PC = " << std::hex << ExecPC << " Inst: " << std::setw( 23 ) << InstTable[Inst.entry].mnemonic
                << " r" << std::dec << (uint32_t) Inst.rd << "= " << std::hex << RegFile->RV32[Inst.rd] << " r" << std::dec
                << (uint32_t) Inst.rs1 << "= " << std::hex << RegFile->RV32[Inst.rs1] << " r" << std::dec << (uint32_t) Inst.rs2
                << "= " << std::hex << RegFile->RV32[Inst.rs2] << " imm = " << std::hex << Inst.imm << std::endl;

    } else {
      std::cout << "RDT: Executed PC = " << std::hex << ExecPC << " Inst: " << std::setw( 23 ) << InstTable[Inst.entry].mnemonic
                << " r" << std::dec << (uint32_t) Inst.rd << "= " << std::hex << RegFile->RV64[Inst.rd] << " r" << std::dec
                << (uint32_t) Inst.rs1 << "= " << std::hex << RegFile->RV64[Inst.rs1] << " r" << std::dec << (uint32_t) Inst.rs2
                << "= " << std::hex << RegFile->RV64[Inst.rs2] << " imm = " << std::hex << Inst.imm << std::endl;
      std::cout << "RDT: Address of RD = 0x" << std::hex << (uint64_t*) ( &RegFile->RV64[Inst.rd] ) << std::dec << std::endl;
    }
#endif

    // inject the ALU fault
    if( ALUFault ) {
      InjectALUFault( EToE, Inst );
    }

    // if this is a singlestep, clear the singlestep and halt
    if( SingleStep ) {
      SingleStep = false;
      Halted     = true;
    }

    rtn = true;
  } else {
    // wait until the counter has been decremented
    // note that this will continue to occur until the counter is drained
    // and the HART is halted
    output->verbose( CALL_INFO, 9, 0, "Core %" PRIu32 " ; No available thread to exec PC= 0x%" PRIx64 "\n", id, ExecPC );
    rtn = true;
    Stats.cyclesIdle_Total++;
    if( HartsClearToExecute.any() ) {
      Stats.cyclesIdle_MemoryFetch++;
    }
  }

  // Check for pipeline hazards
  if( !Pipeline.empty() && ( Pipeline.front().second.cost > 0 ) ) {
    Pipeline.front().second.cost--;
    if( Pipeline.front().second.cost == 0 ) {  // &&
      // Ready to retire this instruction
      uint16_t HartID = Pipeline.front().first;
#ifdef NO_REV_TRACER
      output->verbose(
        CALL_INFO,
        6,
        0,
        "Core %" PRIu32 "; Hart %" PRIu32 "; ThreadID %" PRIu32 "; Retiring PC= 0x%" PRIx64 "\n",
        id,
        HartID,
        ActiveThreadID,
        ExecPC
      );
#endif
      Stats.retired++;

      // Only clear the dependency if there is no outstanding load
      if( ( RegFile->GetLSQueue()->count( LSQHash( Pipeline.front().second.rd, InstTable[Pipeline.front().second.entry].rdClass, HartID ) ) ) == 0 ) {
        DependencyClear( HartID, &( Pipeline.front().second ) );
      }
      Pipeline.pop_front();
      RegFile->SetCost( 0 );
    } else {
      // could not retire the instruction, bump the cost
      Pipeline.front().second.cost++;
    }
  }
  // Check for completion states and new tasks
  if( RegFile->GetPC() == 0x00ull ) {
    // look for more work on the execution queue
    // if no work is found, don't update the PC
    // just wait and spin
    if( HartHasNoDependencies( HartToDecodeID ) ) {
      std::unique_ptr<RevThread> ActiveThread = PopThreadFromHart( HartToDecodeID );
      ActiveThread->SetState( ThreadState::DONE );
      HartsClearToExecute[HartToDecodeID] = false;
      HartsClearToDecode[HartToDecodeID]  = false;
      IdleHarts.set( HartToDecodeID );
      AddThreadsThatChangedState( std::move( ActiveThread ) );
    }

    if( HartToExecID != _REV_INVALID_HART_ID_ && !IdleHarts[HartToExecID] && HartHasNoDependencies( HartToExecID ) ) {
      std::unique_ptr<RevThread> ActiveThread = PopThreadFromHart( HartToDecodeID );
      ActiveThread->SetState( ThreadState::DONE );
      HartsClearToExecute[HartToExecID] = false;
      HartsClearToDecode[HartToExecID]  = false;
      IdleHarts[HartToExecID]           = true;
      AddThreadsThatChangedState( std::move( ActiveThread ) );
    }
  }

#ifndef NO_REV_TRACER
  // Dump trace state
  if( Tracer )
    Tracer->Render( currentCycle );
#endif

  return rtn;
}

std::unique_ptr<RevThread> RevCore::PopThreadFromHart( unsigned HartID ) {
  if( HartID >= numHarts ) {
    output->fatal(
      CALL_INFO, -1, "Error: tried to pop thread from hart %" PRIu32 " but there are only %" PRIu32 " hart(s)\n", HartID, numHarts
    );
  }
  IdleHarts[HartID] = true;
  return Harts.at( HartID )->PopThread();
}

void RevCore::PrintStatSummary() {
  auto memStatsTotal = mem->GetMemStatsTotal();

  double eff         = StatsTotal.totalCycles ? double( StatsTotal.cyclesBusy ) / StatsTotal.totalCycles : 0;
  output->verbose(
    CALL_INFO,
    2,
    0,
    "Program execution complete\n"
    "Core %u Program Stats: Total Cycles: %" PRIu64 " Busy Cycles: %" PRIu64 " Idle Cycles: %" PRIu64 " Eff: %f\n",
    id,
    StatsTotal.totalCycles,
    StatsTotal.cyclesBusy,
    StatsTotal.cyclesIdle_Total,
    eff
  );

  output->verbose(
    CALL_INFO,
    3,
    0,
    "\t Bytes Read: %" PRIu64 " Bytes Written: %" PRIu64 " Floats Read: %" PRIu64 " Doubles Read %" PRIu64 " Floats Exec: %" PRIu64
    " TLB Hits: %" PRIu64 " TLB Misses: %" PRIu64 " Inst Retired: %" PRIu64 "\n\n",
    memStatsTotal.bytesRead,
    memStatsTotal.bytesWritten,
    memStatsTotal.floatsRead,
    memStatsTotal.doublesRead,
    StatsTotal.floatsExec,
    memStatsTotal.TLBHits,
    memStatsTotal.TLBMisses,
    StatsTotal.retired
  );
}

RevRegFile* RevCore::GetRegFile( unsigned HartID ) const {
  if( HartID >= Harts.size() ) {
    output->fatal(
      CALL_INFO, -1, "Error: tried to get RegFile for Hart %" PRIu32 " but there are only %" PRIu32 " hart(s)\n", HartID, numHarts
    );
  }
  return Harts.at( HartID )->RegFile.get();
}

void RevCore::CreateThread( uint32_t NewTID, uint64_t firstPC, void* arg ) {
  // tidAddr is the address we have to write the new thread's id to
  output->verbose( CALL_INFO, 2, 0, "Creating new thread with PC = 0x%" PRIx64 "\n", firstPC );
  uint32_t ParentThreadID                      = Harts.at( HartToExecID )->GetAssignedThreadID();

  // Create the new thread's memory
  std::shared_ptr<MemSegment> NewThreadMem     = mem->AddThreadMem();

  // TODO: Copy TLS into new memory

  // Create new register file
  std::unique_ptr<RevRegFile> NewThreadRegFile = std::make_unique<RevRegFile>( feature );

  // Copy the arg to the new threads a0 register
  NewThreadRegFile->SetX( RevReg::a0, reinterpret_cast<uintptr_t>( arg ) );

  // Set the global pointer
  // TODO: Cleanup
  NewThreadRegFile->SetX( RevReg::tp, NewThreadMem->getTopAddr() );
  NewThreadRegFile->SetX( RevReg::sp, NewThreadMem->getTopAddr() - mem->GetTLSSize() );
  NewThreadRegFile->SetX( RevReg::gp, loader->GetSymbolAddr( "__global_pointer$" ) );
  NewThreadRegFile->SetX( 8, loader->GetSymbolAddr( "__global_pointer$" ) );
  NewThreadRegFile->SetPC( firstPC );

  // Create a new RevThread Object
  std::unique_ptr<RevThread> NewThread =
    std::make_unique<RevThread>( NewTID, ParentThreadID, NewThreadMem, std::move( NewThreadRegFile ) );

  // Add new thread to this vector so the RevCPU will add and schedule it
  AddThreadsThatChangedState( std::move( NewThread ) );

  return;
}

//
// This is the function that is called when an ECALL exception is detected inside ClockTick
// - Currently the only way to set this exception is by Ext->Execute(....) an ECALL instruction
//
// Eventually this will be integrated into a TrapHandler however since ECALLs are the only
// supported exceptions at this point there is no need just yet.
//
// Returns true if an ECALL is in progress
bool RevCore::ExecEcall() {
  if( RegFile->GetSCAUSE() != RevExceptionCause::ECALL_USER_MODE )
    return false;

  // ECALL in progress
  uint32_t EcallCode = RegFile->GetX<uint32_t>( RevReg::a7 );
  output->verbose(
    CALL_INFO,
    6,
    0,
    "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32 " - Exception Raised: ECALL with code = %" PRIu32 "\n",
    id,
    HartToExecID,
    ActiveThreadID,
    EcallCode
  );

  // TODO: Cache handler function during ECALL instruction execution
  auto it = Ecalls.find( EcallCode );
  if( it == Ecalls.end() ) {
    output->fatal( CALL_INFO, -1, "Ecall Code = %" PRIu32 " not found", EcallCode );
  }

  // Execute the Ecall handler
  HartToExecID   = HartToDecodeID;
  bool completed = ( this->*it->second )() != EcallStatus::CONTINUE;

  // If we have completed, reset the EcallState and SCAUSE
  if( completed ) {
    Harts[HartToDecodeID]->GetEcallState().clear();
    RegFile->SetSCAUSE( RevExceptionCause::NONE );
  }

  return true;
}

// Looks for a hart without a thread assigned to it and then assigns it.
// This function should never be called if there are no available harts
// so if for some reason we can't find a hart without a thread assigned
// to it then we have a bug.
void RevCore::AssignThread( std::unique_ptr<RevThread> Thread ) {
  unsigned HartToAssign = FindIdleHartID();

  if( HartToAssign == _REV_INVALID_HART_ID_ ) {
    output->fatal(
      CALL_INFO,
      1,
      "Attempted to assign a thread to a hart but no available harts were "
      "found.\n"
      "We should never have tried to assign a thread to this Proc if it had no "
      "harts available (ie. Proc->NumIdleHarts() == 0 ).\n"
      "This is a bug\n"
    );
  }

  // Assign the thread to the hart
  Harts.at( HartToAssign )->AssignThread( std::move( Thread ) );

  IdleHarts[HartToAssign] = false;

  return;
}

unsigned RevCore::FindIdleHartID() const {
  unsigned IdleHartID = _REV_INVALID_HART_ID_;
  // Iterate over IdleHarts to find the first idle hart
  for( size_t i = 0; i < Harts.size(); i++ ) {
    if( IdleHarts[i] ) {
      IdleHartID = i;
      break;
    }
  }
  if( IdleHartID == _REV_INVALID_HART_ID_ ) {
    output->fatal( CALL_INFO, -1, "Attempted to find an idle hart but none were found. This is a bug\n" );
  }

  return IdleHartID;
}

void RevCore::InjectALUFault( std::pair<unsigned, unsigned> EToE, RevInst& Inst ) {
  // inject ALU fault
  RevExt* Ext = Extensions[EToE.first].get();
  if( ( Ext->GetName() == "RV64F" ) || ( Ext->GetName() == "RV64D" ) ) {
    // write an rv64 float rd
    uint64_t tmp;
    static_assert( sizeof( tmp ) == sizeof( RegFile->DPF[Inst.rd] ) );
    memcpy( &tmp, &RegFile->DPF[Inst.rd], sizeof( tmp ) );
    tmp |= RevRand( 0, ~( ~uint64_t{ 0 } << fault_width ) );
    memcpy( &RegFile->DPF[Inst.rd], &tmp, sizeof( tmp ) );
  } else if( ( Ext->GetName() == "RV32F" ) || ( Ext->GetName() == "RV32D" ) ) {
    // write an rv32 float rd
    uint32_t tmp;
    static_assert( sizeof( tmp ) == sizeof( RegFile->SPF[Inst.rd] ) );
    memcpy( &tmp, &RegFile->SPF[Inst.rd], sizeof( tmp ) );
    tmp |= RevRand( 0, ~( uint32_t{ 0 } << fault_width ) );
    memcpy( &RegFile->SPF[Inst.rd], &tmp, sizeof( tmp ) );
  } else {
    // write an X register
    uint64_t rval = RevRand( 0, ~( ~uint64_t{ 0 } << fault_width ) );
    RegFile->SetX( Inst.rd, rval | RegFile->GetX<uint64_t>( Inst.rd ) );
  }

  // clear the fault
  ALUFault = false;
}

///< RevCore: Used by RevCPU to determine if it can disable this proc
///           based on the criteria there are no threads assigned to it and the
///           CoProc is done
bool RevCore::HasNoWork() const {
  return HasNoBusyHarts() && ( !coProc || coProc->IsDone() );
}

void RevCore::UpdateStatusOfHarts() {
  // A Hart is ClearToDecode if:
  //   1. It has a thread assigned to it (ie. NOT Idle)
  //   2. It's last instruction is done executing (ie. cost is set to 0)
  for( size_t i = 0; i < Harts.size(); i++ ) {
    HartsClearToDecode[i] = !IdleHarts[i] && Harts[i]->RegFile->cost == 0;
  }
  return;
}

}  // namespace SST::RevCPU

// EOF
