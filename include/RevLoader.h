//
// _RevLoader_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVLOADER_H_
#define _SST_REVCPU_REVLOADER_H_

// -- Standard Headers
#include <map>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>

// -- RevCPU Headers
#include "RevMem.h"

#ifndef PT_LOAD
#define PT_LOAD 1
#endif

#ifndef SHT_NOBITS
#define SHT_NOBITS 8
#endif

namespace SST::RevCPU {
  class RevLoader;
}

using namespace SST::RevCPU;

template<typename T> static inline T from_le(T n) { return n; }

namespace SST {
  namespace RevCPU {

    typedef struct{
      uint8_t  e_ident[16];
      uint16_t e_type;
      uint16_t e_machine;
      uint32_t e_version;
      uint32_t e_entry;
      uint32_t e_phoff;
      uint32_t e_shoff;
      uint32_t e_flags;
      uint16_t e_ehsize;
      uint16_t e_phentsize;
      uint16_t e_phnum;
      uint16_t e_shentsize;
      uint16_t e_shnum;
      uint16_t e_shstrndx;
    } Elf32_Ehdr;

    typedef struct{
      uint32_t sh_name;
      uint32_t sh_type;
      uint32_t sh_flags;
      uint32_t sh_addr;
      uint32_t sh_offset;
      uint32_t sh_size;
      uint32_t sh_link;
      uint32_t sh_info;
      uint32_t sh_addralign;
      uint32_t sh_entsize;
    } Elf32_Shdr;

    typedef struct{
      uint32_t p_type;
      uint32_t p_offset;
      uint32_t p_vaddr;
      uint32_t p_paddr;
      uint32_t p_filesz;
      uint32_t p_memsz;
      uint32_t p_flags;
      uint32_t p_align;
    } Elf32_Phdr;

    typedef struct{
      uint32_t st_name;
      uint32_t st_value;
      uint32_t st_size;
      uint8_t  st_info;
      uint8_t  st_other;
      uint16_t st_shndx;
    } Elf32_Sym;

    typedef struct{
      uint8_t  e_ident[16];
      uint16_t e_type;
      uint16_t e_machine;
      uint32_t e_version;
      uint64_t e_entry;
      uint64_t e_phoff;
      uint64_t e_shoff;
      uint32_t e_flags;
      uint16_t e_ehsize;
      uint16_t e_phentsize;
      uint16_t e_phnum;
      uint16_t e_shentsize;
      uint16_t e_shnum;
      uint16_t e_shstrndx;
    } Elf64_Ehdr;

    typedef struct{
      uint32_t sh_name;
      uint32_t sh_type;
      uint64_t sh_flags;
      uint64_t sh_addr;
      uint64_t sh_offset;
      uint64_t sh_size;
      uint32_t sh_link;
      uint32_t sh_info;
      uint64_t sh_addralign;
      uint64_t sh_entsize;
    } Elf64_Shdr;

    typedef struct{
      uint32_t p_type;
      uint32_t p_flags;
      uint64_t p_offset;
      uint64_t p_vaddr;
      uint64_t p_paddr;
      uint64_t p_filesz;
      uint64_t p_memsz;
      uint64_t p_align;
    } Elf64_Phdr;

    typedef struct{
      uint32_t st_name;
      uint8_t  st_info;
      uint8_t  st_other;
      uint16_t st_shndx;
      uint64_t st_value;
      uint64_t st_size;
    } Elf64_Sym;

    typedef struct{
      int phent;
      int phnum;
      int is_supervisor;
      size_t phdr;
      size_t phdr_size;
      size_t bias;
      size_t entry;
      size_t brk_min;
      size_t brk;
      size_t brk_max;
      size_t mmap_max;
      size_t stack_top;
      uint64_t time0;
      uint64_t cycle0;
      uint64_t instret0;
    } ElfInfo;

    class RevLoader {
    public:
      /// RevLoader: standard constructor
      RevLoader( std::string Exe, std::string Args, RevMem *Mem, SST::Output *Output );

      /// RevLoader: standard destructor
      ~RevLoader();

      /// RevLoader: retrieves the address for the target symbol; 0x00ull if the symbol doesn't exist
      uint64_t GetSymbolAddr(std::string Symbol);

      /// RevLoader: retrieves the value for 'argc'
      unsigned GetArgc() { return argv.size(); }

      /// RevLoader: retrieves the target value within the argv array
      std::string GetArgv(unsigned entry);

      /// RevLoader: retrives the elf info structure
      ElfInfo GetInfo() { return elfinfo; }

    private:
      std::string exe;          ///< RevLoader: binary executable
      std::string args;         ///< RevLoader: program args
      RevMem *mem;              ///< RevLoader: memory object
      SST::Output *output;      ///< RevLoader: output handler

      uint32_t RV32Entry;       ///< RevLoader: RV32 entry
      uint64_t RV64Entry;       ///< RevLoader: RV64 entry

      ElfInfo elfinfo;          ///< RevLoader: elf info from the loaded program

      std::map<std::string,uint64_t> symtable;  ///< RevLoader: loaded symbol table

      std::vector<std::string> argv;            ///< RevLoader: The actual argv table

      /// Loads the target executable into memory
      bool LoadElf();

      /// Loads the target program arguments
      bool LoadProgramArgs();

      /// Determines if the target header is an Elf header
      bool IsElf( const Elf64_Ehdr eh64 );

      /// Determines if the target header is an Elf32 header
      bool IsRVElf32( const Elf64_Ehdr eh64 );

      /// Determines if the target header is an Elf64 header
      bool IsRVElf64( const Elf64_Ehdr eh64 );

      /// Determines if the target header is little endian
      bool IsRVLittle( const Elf64_Ehdr eh64 );

      /// Determines if the target header is big endian
      bool IsRVBig( const Elf64_Ehdr eh64 );

      /// Loads a 32bit Elf binary
      bool LoadElf32(char *MemBuf, size_t Size);

      /// Loads a 64bit Elf binary
      bool LoadElf64(char *MemBuf, size_t Size);

      ///< Splits a string into tokens
      void splitStr(const std::string& s,char c,std::vector<std::string>& v);

      ///< Breaks bulk writes into cache lines
      bool WriteCacheLine(uint64_t Addr, size_t Len, void *Data);

    }; // class Loader
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
