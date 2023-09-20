# Extension Development

## Overview
The Rev SST component permits users to create unique extensions that are not defined as a standard extension by the RISC-V standards body.  This allows users to experiment with features that may be advanced or override default extension behavior.  Further, this also allows users to experiment with accelerators that may be orthogonal to the base RISC-V ISA.  Each RISC-V extension is implemented in Rev using a single header file.  The header file contains the instruction encoding table and an implementation function for each individual instruction contained within the extension.  

There are also several limitations of the current extension functionality.  These limitations can be overcome, but will require additional source code modifications.  These limitations are noted as follows:

1. The Rev crack/decode functions currently only support the standard set of RISC-V instruction formats.  Additional formats can be supported, but will require additional source code modifications to the crack/decode engine.
2. The Rev model supports the ability for users to override default extensions (for example, the D-extension) with new functionality.  However, you cannot load two extensions with conflicting encodings.  This will break the Rev crack/decode engine.  
3. The naming convention for the new extension cannot conflict with the standard set of the extensions if they are utilized in the same core.  Eg, an RV64IMAFD device cannot have a second "F" extension.  You must utilize a different naming convention.
4. The Rev model utilizes a standard ELF loader.  If the extension breaks the base RISC-V (RV32, RV64) relocation, then the device may not function as expected.
5. The Rev model supports the standard set of RV32/RV64-G registers for integer and floating point.  Any extension-specific register state will require additional source code modifications.

## Source Code Organization

From the base Rev directory, all the source code resides in `src`.  The instruction implementation header files reside in `src/insns`.  Additional source files of interest are noted as follows:

| File | Description |
| ----------- | ----------- |
| `src/RevInstTables.h` | Includes all the instruction implementation headers |
| `src/RevInstTable.h` | Contains the base strucutures utilized to create each extension as well as functions to assist in instruction implementation. |
| `src/RevFeature.h` | Contains the feature to extension mappings. |
| `src/RevMem.h` | Contains all the interfaces for reading/writing memory. |
| `src/RevProc.cc` | Contains the main simulation driver and instruction table loader. |

## Documentation

The Rev source tree utilizes Doxygen style comments for documentaton.  This includes the individual extension implementation headers.  Each variable should be documentated using the `///<` comment and each function should be documented with the three-slash comment `///`.  Initiating `make doc` will automatically build the documentation.

## Adding an Extension

### Choose an Extension Mnemonic

Each extension requires a notional mnemonic that can be parsed by the Rev infrastructure and recognized as a supported extension.  It is generally good practice to choose an unused or unsupport letter for your extension.  In this case, we choose the letter `Z` to represent our sample extension.  

### Add the Mnemonic to the RevFeature Handlers

Now that we have a mnemonic defined, we need to add the support for the new extension in the `RevFeature` class.

The first step in doing so is to added an entry in the `RevFeatureType` enumerated type list to represent your extension.  This can be found in the `RevFeature.h` header.  Make sure that you choose a unique power of 2, since the features are ORed together. Right now the list includes all feature strings defined in the RISC-V Unproviledged Specification, Chapter 27.

```c++
enum RevFeatureType : uint32_t {
    RV_UNKNOWN  = 0,      ///< RevFeatureType: unknown feature
    RV_E        = 1<<0,   ///< RevFeatureType: E-extension
    RV_I        = 1<<1,   ///< RevFeatureType: I-extension
    RV_M        = 1<<2,   ///< RevFeatureType: M-extension
    RV_A        = 1<<3,   ///< RevFeatureType: A-extension
    RV_F        = 1<<4,   ///< RevFeatureType: F-extension
    RV_D        = 1<<5,   ///< RevFeatureType: D-extension
    RV_Q        = 1<<6,   ///< RevFeatureType: Q-extension
    RV_L        = 1<<7,   ///< RevFeatureType: L-extension
    RV_C        = 1<<8,   ///< RevFeatureType: C-extension
    RV_B        = 1<<9,   ///< RevFeatureType: B-extension
    RV_J        = 1<<10,  ///< RevFeatureType: J-extension
    RV_T        = 1<<11,  ///< RevFeatureType: T-extension
    RV_P        = 1<<12,  ///< RevFeatureType: P-Extension
    RV_V        = 1<<13,  ///< RevFeatureType: V-extension
    RV_N        = 1<<14,  ///< RevFeatureType: N-extension
    RV_ZICSR    = 1<<15,  ///< RevFEatureType: Zicsr-extension
    RV_ZIFENCEI = 1<<16,  ///< RevFeatureType: Zifencei-extension
    RV_ZAM      = 1<<17,  ///< RevFeatureType: Zam-extension
    RV_ZTSO     = 1<<18,  ///< RevFeatureType: Ztso-extension
    RV_ZFA      = 1<<19,  ///< RevFeatureType: Zfa-extension
    };
```

Now we need to add support in the extension parser.  This resides in the `ParseMachineModel` function inside `RevFeature.cc`.

This function a loop over the device string and adds the necessary support for the specific extension(s).

Adding support for new features is as simple as adding a new entry into the machine model table as folows.
Notice how we utilize the `SetMachineEntry` function with the appropriate enumerated type defined above.  

```c++
  ///< List of architecture extensions. These must listed in canonical order
  ///< as shown in Table 27.11, Chapter 27, of the RiSC-V Unpriviledged Spec.
  ///< By using a canonical ordering, the extensions' presence can be tested
  ///< in linear time complexity of the table and the string. Some of the
  ///< extensions imply other extensions, so the extension flags are ORed.
  static constexpr std::pair<std::string_view, uint32_t> table[] = {
    { "E",          RV_E                                                      },
    { "I",          RV_I                                                      },
    { "M",          RV_M                                                      },
    { "A",          RV_A                                                      },
    { "F",          RV_F | RV_ZICSR                                           },
    { "D",          RV_D | RV_F | RV_ZICSR                                    },
    { "G",          RV_I | RV_M | RV_A | RV_F | RV_D | RV_ZICSR | RV_ZIFENCEI },
    { "Q",          RV_Q | RV_D | RV_F | RV_ZICSR                             },
    { "L",          RV_L                                                      },
    { "C",          RV_C                                                      },
    { "B",          RV_B                                                      },
    { "J",          RV_J                                                      },
    { "T",          RV_T                                                      },
    { "P",          RV_P                                                      },
    { "V",          RV_V | RV_D | RV_F | RV_ZICSR                             },
    { "N",          RV_N                                                      },
    { "Zicsr",      RV_ZICSR                                                  },
    { "Zifencei",   RV_ZIFENCEI                                               },
    { "Zam",        RV_ZAM | RV_A                                             },
    { "Ztso",       RV_ZTSO                                                   },
    { "Zfa",        RV_ZFA | RV_F | RV_ZICSR                                  },
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
```
### Add the Extension Header to the RevInstTables Header

Now that you've defined your new extension mnemonic, you need to add an entry in the `RevInstTables.h` header 
file such that the remainder of the Rev infrastructure can find the new implementation details.  The header 
name that you choose here must also be utilized in the next step.

As an example, we create the `Z` extension and add the `RV32Z.h` header file.

```c++
//
// _RevInstTables_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVINSTTABLES_H_
#define _SST_REVCPU_REVINSTTABLES_H_

//
// RevInstTables
//
// This file includes all the child instruction
// table header files that define the encodings
// and implementation for each block of RISC-V isntructions
//

#include "insns/RV32I.h"
#include "insns/RV64I.h"
#include "insns/RV32M.h"
#include "insns/RV64M.h"
#include "insns/RV32A.h"
#include "insns/RV64A.h"
#include "insns/RV32F.h"
#include "insns/RV64F.h"
#include "insns/RV32D.h"
#include "insns/RV64D.h"

// Our new extension
#include "insns/RV32Z.h"

#endif
```

### Add the Instruction Table Loader
In this section, we need to add support for loading the new extension's instructions into the internal Rev instruction table.  Rev utilizes an internal instruction table with compressed encodings in order to permit rapid crack/decode.  Each table entry contains a pointer to the respective implementation function for the target instruction.  In this case, we need to add the necessary logic to 1) detect that our new extension is enabled and 2) add the associated instructions to the internal instruction table.

For this, we need to modify the `RevProc.cc` implementation file.  Specifically, we will be modifying the contents of the `SeedInstTable` function.  Each new instruction implementation object is statically cast to the base `RevExt` type and passed to the `EnableExt` function.  An example of adding the `Z` extension is as follows.  Also note that the newly created `RV32Z` object is given the feature object, a pointer to the register file, the memory object and the SST output object.  

```c++
bool RevProc::SeedInstTable(){
  // Z-Extension
  if( feature->IsModeEnabled(RV_Z) ){
    EnableExt(static_cast<RevExt *>(new RV32Z(feature,&RegFile,mem,output)));
  }

  // I-Extension
  if( feature->IsModeEnabled(RV_I) ){
    EnableExt(static_cast<RevExt *>(new RV32I(feature,&RegFile,mem,output)));
    if( feature->GetXlen() == 64 ){
      EnableExt(static_cast<RevExt *>(new RV64I(feature,&RegFile,mem,output)));
    }
  }
...
```

### Create the Extension Header
The final series of steps to create a new extension is where the bulk of the code will reside.  As we stated above, each implementation includes a unique header file that provides the instruction implementations and encoding tables for the target extension.  In this section, we will create the header file and add several basic instructions.  This will elicit how we 1) construct the instruction tables, 2) create instruction implementations and 3) utilize the provided functions to perform basic memory and arithmetic operations.  

The first thing we need to do is create the basic header file at `src/insns/RV32Z.h`.  The basic skeleton of this header will resemble the following: 

```c++
//
// _RV32Z_h_
//
// Copyright info
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32Z_H_
#define _SST_REVCPU_RV32Z_H_

#include "RevInstTable.h"
#include "RevExt.h"

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{
    struct RV32Z : RevExt {

    // RV32Z Implementation Functions

    // RV32Z Instruction Table

      /// RV32Z: standard constructor
      RV32Z( RevFeature *Feature,
             RevRegFile *RegFile,
             RevMem *RevMem,
             SST::Output *Output )
        : RevExt( "RV32Z", Feature, RegFile, RevMem, Output ) {
          SetTable(RV32ZTable);
        }

      /// RV32Z: standard destructor
      ~RV32Z() = default;

    }; // end class RV32I
  } // namespace RevCPU
} // namespace SST
```

There are a few important things we need to point out before we move on. First,
notice how the new implementation class resides inside the `SST::RevCPU`
namespace and inherits functions from the base `RevExt` class. This is very
important in order to correctly load your new instructions into the simulator.
Second, notice how we instantiate the constructor for the new extension. The
constructor **MUST** contain a call to `SetTable(RV32ZTable)`. Given that this
is a header-only implementation, the order of which the class constructors and
associated data members must be preserved. This method forces the child
constructor to create its private data members before registering them with the
base class.

Now that we have our basic skeleton in place, we can start creating our
instruction table. The table is actually a C++ vector of struct entries where
each entry corresponds to a single instruction entry. This needs to be done in
the private section of the class (see the comment above for RV32Z Instruction
Table). The stuct for each entry is created by an in-line creation of a
`RevInstEntryBuilder< >` object. This object will utilize the class declared in
the template parameter for the default values. The base default values can be
found in the `RevInstDefaults` class and can be overriden through inheritence
(shown in the example below). The individual elements of the `RevInstEntry`
struct are initialized using named arguments so they can be initialized in any
order. It is important to end the argument initialization chain with
`.InstEntry` as this is the actual struct that will be added to the
`std::vector`.First, lets create a few basic entries in our table, then we'll
explain what each entry is used for.

```c++
struct Rev32ZInstDefaults : RevInstDefaults {
  RevRegF format = RVTypeR; 
}
std::vector<RevInstEntry> RV32ZTable = {
{RevInstEntryBuilder<Rev32ZInstDefaults>().SetMnemonic("zadd %rd, %rs1, %rs2").SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0000000).
   SetrdClass(RegGPR).Setrs1Class(RegGPR)Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetImplFunc(&zadd).InstEntry },
{RevInstEntryBuilder<Rev32ZInstDefaults>().SetMnemonic("zsub %rd, %rs1, %rs2").SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0100000).
   SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetImplFunc(&zsub).InstEntry },
{RevInstEntryBuilder<Rev32ZInstDefaults>().SetMnemonic("zlb %rd, $imm(%rs1)").SetCost(1).SetOpcode(0b0000011).SetFunct3(0b000).SetFunct7(0b0).
   SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&zlb).InstEntry },
{RevInstEntryBuilder<Rev32ZInstDefaults>().SetMnemonic("zsb %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b000).SetFunct7(0b0).
   SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&zsub).InstEntry },
};
```

For this, we've created four instructions: `zadd`, `zsub`, `zlb` and `zsb` to represent two arithmetic instructions a load instruction and a store instruction.  Each field in the entry have specific values associated with them.  The field entries are outlined (in order) as follows.  Please be careful with entering data into the tables as the data contained therein drives the crack/decode and execution of the core simulation.  

| Field Num | Field | Description |
| ----------- | ----------- | ----------- |
| 1 | mnemonic | This describes how to *type* the instruction.  This is a specical syntax that can utilized during debugging or disassembly.  Register fields are noted using percent signs and their field name and immediate fields are noted using the dollar sign and their field name.  Ex: `add %rd, %rs1, %rs2` or `lb %rd, $imm(%rs1)` |
| 2 | cost | This is a nonzero value that represents the cost (in clock cycles) of the default instruction implementation.  This will determine how many cycles this instruction will execute prior to being retired.  This value can be overridden by the user at runtime. |
| 3 | opcode | This is the seven bit opcode of the instruction. |
| 4 | funct3 | This is the funct3 encoding field.  If the respective instruction does not utilize the field, set this value to `0b000` |
| 5 | funct7 | This is the funct7 encoding field.  If the respective instruction does not utilize the field, set this value to `0b0000000` |
| 6 | rdClass | If the instruction has an `rd` register slot, this denotes the register class utilized.  Values for this can be one of `RegGPR` for the general purpose register file, `RegCSR` for the CSR register file, `RegFloat` for the floating point register file, `RegIMM` (treat the reg class like an immediate, only utilized in the S-format) or `RegUNKNOWN` if the field is not utilized. |
| 7 | rs1Class | Defines the register class for the `rs1` slot.  Use `RegUNKNOWN` if this slot is not utilized | 
| 8 | rs2Class | Defines the register class for the `rs2` slot.  Use `RegUNKNOWN` if this slot is not utilized | 
| 9 | rs3Class | Defines the register class for the `rs3` slot.  Use `RegUNKNOWN` if this slot is not utilized | 
| 10 | imm12 | Defines the value of the `imm12` slot if the immediate is hardwired to a single value. |
| 11 | imm | Defines the functionality of th `imm12` field.  If the field is not used, set this to `Funk`.  `FImm` indicates that the field is present and utilized, `FEnc` indicates that this field is an encoding value and `FVal` is an incoming register value.  When using `FEnc`, the `imm12` entry (10) must also be set. |
| 12 | format | Defines the instruction format.  This is one of: `RVTypeUNKNOWN`, `RVTypeR`, `RVTypeI`, `RVTypeS`, `RVTypeU`, `RVTypeB`, `RVTypeJ` or `RVTypeR4` |
| 13 | func | This contains a function pointer to the implementation function of the target instruction |

Now that we have our instruction encoding tables, we can begin implementing each of our instruction functions in the private section of the header file.  Note that this must be done **above** the instruction table as the symbol names must be defined prior to their use in the instruction table.  First, we'll show an example implementation of our four instructions before outlining all the requirements and features.

### Helper Functions
There are several helper functions to make generating instruction implementations easier.

```c++
  /// Zero-extend value of bits size
  template<typename T>
  constexpr auto ZeroExt(T val, size_t bits);
```
```c++
  /// Sign-extend value of bits size
  template<typename T>
  constexpr auto SignExt(T val, size_t bits);
```
```c++
  /// BoxNaN: Store a boxed float inside a double
  inline void BoxNaN(double* dest, const void* value);
```
```c++
  /// GetX: Get the specifed X register cast to a specific integral type
  template<typename T>
  T GetX(const RevFeature* F, size_t rs) const;
```
```c++
  /// SetX: Set the specifed X register to a specific value
  template<typename T>
  void SetX(const RevFeature* F, size_t rd, T val);
```
```c++
  /// GetPC: Get the Program Counter
  uint64_t GetPC(const RevFeature* F) const;
```
```c++
  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC(const RevFeature* F, T val);
```
```c++
  /// AdvancePC: Advance the program counter a certain number of bytes
  template<typename T>
  void AdvancePC(const RevFeature* F, T bytes);
```
```c++
  /// GetFP32: Get the 32-bit float value of a specific FP register
  float GetFP32(const RevFeature* F, size_t rs) const;
```
```c++
  /// SetFP32: Set a specific FP register to a 32-bit float value
  void SetFP32(const RevFeature* F, size_t rd, float value);
```
```c++
  /// RevInst: Sign-extended immediate value
  constexpr int32_t ImmSignExt(size_t bits) const;
```
```c++
/// General template for converting between Floating Point and Integer.
/// FP values outside the range of the target integer type are clipped
/// at the integer type's numerical limits, whether signed or unsigned.
template<typename FP, typename INT>
bool CvtFpToInt(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// fclass: Return FP classification like the RISC-V fclass instruction
// See: https://github.com/riscv/riscv-isa-sim/blob/master/softfloat/f32_classify.c
// Because quiet and signaling NaNs are not distinguished by the C++ standard,
// an additional argument has been added to disambiguate between quiet and
// signaling NaNs.
template<typename T>
unsigned fclass(T val, bool quietNaN = true);
```
```c++
/// Load template
template<typename T>
bool load(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Store template
template<typename T>
bool store(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Floating-point load template
template<typename T>
bool fload(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Floating-point store template
template<typename T>
bool fstore(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Floating-point operation template
template<typename T, template<class> class OP>
bool foper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Floating-point minimum functor
template<typename = void>
struct FMin;
```
```c++
/// Floating-point maximum functor
template<typename = void>
struct FMax;
```
```
/// Floating-point conditional operation template
template<typename T, template<class> class OP>
bool fcondop(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Arithmetic operator template
// The First parameter is the operator functor (such as std::plus)
// The second parameter is the operand kind (OpKind::Imm or OpKind::Reg)
// The third parameter is std::make_unsigned_t or std::make_signed_t (default)
// The optional fourth parameter indicates W mode (32-bit on XLEN == 64)
template<template<class> class OP, OpKind KIND,
         template<class> class SIGN = std::make_signed_t, bool W_MODE = false>
bool oper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```
```c++
/// Left shift functor
template<typename = void>
struct ShiftLeft;
```
```c++
/// Right shift functor
template<typename = void>
struct ShiftRight;
```
```c++
enum class DivRem { Div, Rem };
```
```c++
/// Division/Remainder template
// The first parameter is DivRem::Div or DivRem::Rem
// The second parameter is std::make_signed_t or std::make_unsigned_t
// The optional third parameter indicates W mode (32-bit on XLEN == 64)
template<DivRem DIVREM, template<class> class SIGN, bool W_MODE = false>
bool divrem(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);
```

Many instructions can be implemented as references to template functions,
such as:

```c++
  // Arithmetic operators
  static constexpr auto& add   = oper<std::plus,    OpKind::Reg>;
  static constexpr auto& addi  = oper<std::plus,    OpKind::Imm>;
  static constexpr auto& sub   = oper<std::minus,   OpKind::Reg>;
  static constexpr auto& f_xor = oper<std::bit_xor, OpKind::Reg>;
  static constexpr auto& xori  = oper<std::bit_xor, OpKind::Imm>;
  static constexpr auto& f_or  = oper<std::bit_or,  OpKind::Reg>;
  static constexpr auto& ori   = oper<std::bit_or,  OpKind::Imm>;
  static constexpr auto& f_and = oper<std::bit_and, OpKind::Reg>;
  static constexpr auto& andi  = oper<std::bit_and, OpKind::Imm>;

  // Boolean test and set operators
  static constexpr auto& slt   = oper<std::less,    OpKind::Reg>;
  static constexpr auto& slti  = oper<std::less,    OpKind::Imm>;
  static constexpr auto& sltu  = oper<std::less,    OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sltiu = oper<std::less,    OpKind::Imm, std::make_unsigned_t>;

  // Shift operators
  static constexpr auto& slli = oper<ShiftLeft,     OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srli = oper<ShiftRight,    OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srai = oper<ShiftRight,    OpKind::Imm>;
  static constexpr auto& sll  = oper<ShiftLeft,     OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& srl  = oper<ShiftRight,    OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sra  = oper<ShiftRight,    OpKind::Reg>;
```

These define references to function template instantiations, which
can then be used just like the names of ordinary functions.

### Implementing the instruction

```c++
static bool zadd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  if( F->IsRV32() ){
    R->SetX(F, Inst.rd, R->GetX<uint32_t>(F, Inst.rs1) + R->GetX<uint32_t>(F, Inst.rs2));
  }else{
    R->SetX(F, Inst.rd, R->GetX<uint64_t>(F, Inst.rs1) + R->GetX<uint64_t>(F, Inst.rs2));
  }
  R->AdvancePC(F, Inst.instSize);
  return true;
}
```
```c++
static bool zsub(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
  if( F->IsRV32() ){
    R->SetX(F, Inst.rd, R->GetX<uint32_t>(F, Inst.rs1) - R->GetX<uint32_t>(F, Inst.rs2));
  }else{
    R->SetX(F, Inst.rd, R->GetX<uint64_t>(F, Inst.rs1) - R->GetX<uint64_t>(F, Inst.rs2));
  }
  R->AdvancePC(F, Inst.instSize);
  return true;
}

static bool zlb(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
  if( F->IsRV32() ){
    M->ReadVal(F->GetHart(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               reinterpret_cast<int8_t*>(&R->RV32[Inst.rd]),
               Inst.hazard,
               RevCPU::REvFlag::F_SEXT32);
    R->SetX(F, Inst.rd, static_cast<int8_t>(R->RV32[Inst.rd]));
  }else{
    M->ReadVal(F->GetHart(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               reinterpret_cast<int8_t*>(&R->RV64[Inst.rd]),
               Inst.hazard,
               RevCPU::REvFlag::F_SEXT32);
    R->SetX(F, Inst.rd, static_cast<int8_t>(R->RV64[Inst.rd]));
  }
  R->AdvancePC(F, Inst.instSize);
  // update the cost
  R->cost += M->RandCost(F->GetMinCost(),F->GetMaxCost());
  return true;
}

static bool zsb(RevFeature *F, RevRegFile *R,RevMem *M,RevInst Inst) {
  M->Write(F->GetHart(),
           R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
           R->GetX<uint8_t>(F, Inst.rs2));
  R->AdvancePC(F, Inst.instSize);
  return true;
}
```
Or, with the helper functions:
```c++
  static constexpr auto& zadd = oper<std::plus,  OpKind::Reg>;
  static constexpr auto& zadd = oper<std::minus, OpKind::Reg>;
  static constexpr auto& zlb  = load<int8_t>;
  static constexpr auto& zsb  = store<int8_t>;
```

As we can see from the source code above, each function must be formatted as: `static bool FUNC(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst)`.  All instructions carry the same arguments.  The first thing to note is the ability to use the `RevFeature` object to query the device architecture.  The Rev model stores register state in different logical storage for RV32 and RV64.  As a result, if your extension supports both variations of *XLEN*, then its often useful to query the loaded features to see which register file to manipulate, but the `R->GetX<T>(F, reg)` and `R->SetX(F, reg, value) functions can automate this.

The second and third arguments represent the register file object and the memory object, respectively.  These objects permit the user to access internal register state and read/write memory.  Finally, the `RevInst` structure contains all the decoded state from the instruction.  This includes all the opcode and function codes as well as each of the encoded register values.  This structure also contains the floating point rounding mode information.  For more information on the exacting contents and their respective data types, see the `RevInstTable.h` header file.  

Now that we've decoded the necessary state and the simulation execution engine launches the function, we can start executing the target arithmetic.  For example, in the `zadd` function, we seek to add two unsigned integers of *XLEN* size.  Normally, this could be achieved using a simple `Rd = Rs1 + Rs2`.  However, recall from the RISC-V specification that arithmetic is performed in two's complement form.  As a result, we must utilize some utility functions to convert to/from two's complement form.  The `td_u32` and `td_u64` functions convert a value *from* two's complement *to* decimal form.  The `dt_u32` and `dt_u64` convert values from decimal form to two's complement.  As you can see in the `zadd` and `zsub` functions, we utilize the `Inst` payload to decode the register indices, the `RevRegFile` structure to retrieve the necesary register value and the `td_u32/64` functions to convert to decimal form.  We then perform the arithmetic, convert the value back to two's complement form and write it back to the register file.  The final step in the basic arithmetic functions is incrementing the PC.  The PC can be manually manipulated (eg, for branch operations), but this is normally done by incrementing the PC by the size of the instruction payload (in bytes).

In the next functions, `zlb` and `zsb` we seek to load and store data to memory.  Just as we did above, we need to convert the input values to decimal form in order to perform the necessary address arithmetic.  We then utilize the `RevMem` object to write the desired number of bytes or read the desired number of bytes via the `ReadU8` and `WriteU8` routines.  The `RevMem` object provides a number of standard interfaces for writing common data types, arbitrary data and performing load reserve/store conditional operations.  Also note the use of the `SEXT` macro.  This performs sign extension on the incoming load value.  The infrastructure also provides a `ZEXT` macro for zero extension.  

Finally, it is important to note the use of the `M->RandCost()` function.  Typically, RISC-V processor implementations do not hazard on memory store operations given the inherent weak memory ordering (or TSO).  However, for load operations, the processor is required to flag a hazard in order to ensure that the data returns before it is utilized in subsquent operations.  The `RandCost()` function provides the simulator the ability to add an arbitrary cost to load operations that is randomly generated in the range of `F->GetMinCost()` and `F->GetMaxCost()`.  These values are set at runtime by the user in the SST Python script.  In this manner, each load operation will generate a random *cost* and set its respective cost (in cycles).  

## Contributions
We welcome outside contributions from corporate, acaddemic and individual developers.  However, 
there are a number of fundamental ground rules that you must adhere to in order to participate.  These 
rules are outlined as follows:

* All code must adhere to the existing C++ coding style.  While we are somewhat flexible in basic style, you will 
adhere to what is currently in place.  This includes camel case C++ methods and inline comments.  Uncommented, 
complicated algorithmic constructs will be rejected.
* We support compilaton and adherence to C++17 methods.
* All new methods and variables contained within public, private and protected class methods must be commented 
using the existing Doxygen-style formatting.  All new classes must also include Doxygen blocks in the new header 
files.  Any pull requests that lack these features will be rejected.
* All changes to functionality and/or the API infrastructure must be accompanied by complementary tests
* All external pull requests **must** target the *devel* branch.  No external pull requests will be accepted 
to the master or main branches.
* All external pull requests must contain sufficient documentation in the pull request comments in order to 
be accepted.
* All pull requests will be reviewed by the core development staff.  Any necessary changes will be marked
in the respective pull request comments.  All pull requests will be tested against in the Tactical 
Computing Laboratories development infrastructure.  This includes tests against all supported platforms.  
Any failures in the test infrastructure will be noted in the pull request comments.

## Authors
* *John Leidel* - *Chief Scientist* - [Tactical Computing Labs](http://www.tactcomplabs.com)
* *Lee Killough* - *Senior Principal Research Engineer* - [Tactical Computing Labs](http://www.tactcomplabs.com)
