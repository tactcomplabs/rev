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
#include "SST.h"

// -- RevCPU Headers
#include "RevMem.h"

#ifndef PT_LOAD
#define PT_LOAD 1
#endif

#ifndef SHT_NOBITS
#define SHT_NOBITS 8
#endif

/* Legal values for e_type (object file type).  */
#define ET_NONE         0               /* No file type */
#define ET_REL          1               /* Relocatable file */
#define ET_EXEC         2               /* Executable file */
#define ET_DYN          3               /* Shared object file */
#define ET_CORE         4               /* Core file */
#define ET_NUM          5               /* Number of defined types */
#define ET_LOOS         0xfe00          /* OS-specific range start */
#define ET_HIOS         0xfeff          /* OS-specific range end */
#define ET_LOPROC       0xff00          /* Processor-specific range start */
#define ET_HIPROC       0xffff          /* Processor-specific range end */

/* Legal values for e_machine (architecture).  */
#define EM_NONE          0      /* No machine */
#define EM_RISCV        243     /* RISC-V */

/* Legal values for e_version (version).  */
#define EV_NONE         0               /* Invalid ELF version */
#define EV_CURRENT      1               /* Current version */
#define EV_NUM          2

/* Special section indices.  */
#define SHN_UNDEF       0               /* Undefined section */
#define SHN_LORESERVE   0xff00          /* Start of reserved indices */
#define SHN_LOPROC      0xff00          /* Start of processor-specific */
#define SHN_HIPROC      0xff1f          /* End of processor-specific */
#define SHN_ABS         0xfff1          /* Associated symbol is absolute */
#define SHN_COMMON      0xfff2          /* Associated symbol is common */
#define SHN_XINDEX      0xffff          /* Index is in extra table.  */
#define SHN_HIRESERVE   0xffff          /* End of reserved indices */

/* Legal values for sh_type (section type).  */
#define SHT_NULL          0             /* Section header table entry unused */
#define SHT_PROGBITS      1             /* Program data */
#define SHT_SYMTAB        2             /* Symbol table */
#define SHT_STRTAB        3             /* String table */
#define SHT_RELA          4             /* Relocation entries with addends */
#define SHT_HASH          5             /* Symbol hash table */
#define SHT_DYNAMIC       6             /* Dynamic linking information */
#define SHT_NOTE          7             /* Notes */
#define SHT_NOBITS        8             /* Program space with no data (bss) */
#define SHT_REL           9             /* Relocation entries, no addends */
#define SHT_SHLIB         10            /* Reserved */
#define SHT_DYNSYM        11            /* Dynamic linker symbol table */
#define SHT_INIT_ARRAY    14            /* Array of constructors */
#define SHT_FINI_ARRAY    15            /* Array of destructors */
#define SHT_PREINIT_ARRAY 16            /* Array of pre-constructors */
#define SHT_GROUP         17            /* Section group */
#define SHT_SYMTAB_SHNDX  18            /* Extended section indices */
#define SHT_NUM           19            /* Number of defined types.  */
#define SHT_LOOS          0x60000000    /* Start OS-specific.  */
#define SHT_GNU_ATTRIBUTES 0x6ffffff5   /* Object attributes.  */
#define SHT_GNU_HASH      0x6ffffff6    /* GNU-style hash table.  */
#define SHT_GNU_LIBLIST   0x6ffffff7    /* Prelink library list */
#define SHT_GNU_verdef    0x6ffffffd    /* Version definition section.  */
#define SHT_GNU_verneed   0x6ffffffe    /* Version needs section.  */
#define SHT_GNU_versym    0x6fffffff    /* Version symbol table.  */
#define SHT_HIOS          0x6fffffff    /* End OS-specific type */
#define SHT_LOPROC        0x70000000    /* Start of processor-specific */
#define SHT_HIPROC        0x7fffffff    /* End of processor-specific */
#define SHT_LOUSER        0x80000000    /* Start of application-specific */
#define SHT_HIUSER        0x8fffffff    /* End of application-specific */

/* Legal values for sh_flags (section flags).  */
#define SHF_WRITE            (1 << 0)   /* Writable */
#define SHF_ALLOC            (1 << 1)   /* Occupies memory during execution */
#define SHF_EXECINSTR        (1 << 2)   /* Executable */
#define SHF_MERGE            (1 << 4)   /* Might be merged */
#define SHF_STRINGS          (1 << 5)   /* Contains nul-terminated strings */
#define SHF_INFO_LINK        (1 << 6)   /* `sh_info' contains SHT index */
#define SHF_LINK_ORDER       (1 << 7)   /* Preserve order after combining */
#define SHF_OS_NONCONFORMING (1 << 8)   /* Non-standard OS specific handling required */
#define SHF_GROUP            (1 << 9)   /* Section is member of a group.  */
#define SHF_TLS              (1 << 10)  /* Section hold thread-local data.  */
#define SHF_COMPRESSED       (1 << 11)  /* Section with compressed data. */
#define SHF_MASKOS           0x0ff00000 /* OS-specific.  */
#define SHF_MASKPROC         0xf0000000 /* Processor-specific */
#define SHF_GNU_RETAIN       (1 << 21)  /* Not to be GCed by linker.  */

/* Legal values for p_type (segment type).  */
#define PT_NULL         0               /* Program header table entry unused */
#define PT_LOAD         1               /* Loadable program segment */
#define PT_DYNAMIC      2               /* Dynamic linking information */
#define PT_INTERP       3               /* Program interpreter */
#define PT_NOTE         4               /* Auxiliary information */
#define PT_SHLIB        5               /* Reserved */
#define PT_PHDR         6               /* Entry for header table itself */
#define PT_TLS          7               /* Thread-local storage segment */
#define PT_NUM          8               /* Number of defined types */
#define PT_LOOS         0x60000000      /* Start of OS-specific */
#define PT_GNU_EH_FRAME 0x6474e550      /* GCC .eh_frame_hdr segment */
#define PT_GNU_STACK    0x6474e551      /* Indicates stack executability */
#define PT_GNU_RELRO    0x6474e552      /* Read-only after relocation */
#define PT_GNU_PROPERTY 0x6474e553      /* GNU property */
#define PT_LOSUNW       0x6ffffffa
#define PT_SUNWBSS      0x6ffffffa      /* Sun Specific segment */
#define PT_SUNWSTACK    0x6ffffffb      /* Stack segment */
#define PT_HISUNW       0x6fffffff
#define PT_HIOS         0x6fffffff      /* End of OS-specific */
#define PT_LOPROC       0x70000000      /* Start of processor-specific */
#define PT_HIPROC       0x7fffffff      /* End of processor-specific */

/* Legal values for p_flags (segment flags).  */

#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
#define PF_MASKOS       0x0ff00000      /* OS-specific */
#define PF_MASKPROC     0xf0000000      /* Processor-specific */



/* Section group handling.  */
#define GRP_COMDAT      0x1             /* Mark group as COMDAT.  */

//template<typename T> static inline T from_le(T n) { return n; }

namespace SST::RevCPU{

struct Elf32_Ehdr{
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
};

struct Elf32_Shdr{
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
};

struct Elf32_Phdr{
  uint32_t p_type;
  uint32_t p_offset;
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz;
  uint32_t p_memsz;
  uint32_t p_flags;
  uint32_t p_align;
};

struct Elf32_Sym{
  uint32_t st_name;
  uint32_t st_value;
  uint32_t st_size;
  uint8_t  st_info;
  uint8_t  st_other;
  uint16_t st_shndx;
};

struct Elf64_Ehdr{
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
};

struct Elf64_Shdr{
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
};

struct Elf64_Phdr{
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
};

struct Elf64_Sym{
  uint32_t st_name;
  uint8_t  st_info;
  uint8_t  st_other;
  uint16_t st_shndx;
  uint64_t st_value;
  uint64_t st_size;
};

struct ElfInfo{
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
};

class RevLoader {
public:
  /// RevLoader: standard constructor
  RevLoader( std::string Exe, std::string Args, RevMem *Mem, SST::Output *Output );

  /// RevLoader: standard destructor
  ~RevLoader();

  /// RevLoader: retrieves the address for the target symbol; 0x00ull if the symbol doesn't exist
  uint64_t GetSymbolAddr(std::string Symbol);

  /// RevLoader: retrieves the value for 'argc'
  auto GetArgc() { return argv.size(); }

  /// RevLoader: retrieves the target value within the argv array
  std::string GetArgv(unsigned entry);

  /// RevLoader: retrieve the entire argv vector
  std::vector<std::string> GetArgv() { return argv; }

  /// RevLoader: retrieves the elf info structure
  ElfInfo GetInfo() { return elfinfo; }

  ///  RevLoader: symbol lookup for tracer
  std::map<uint64_t,std::string>* GetTraceSymbols(); 

  /// RevLoader: Gets TLS base address
  const uint64_t& GetTLSBaseAddr() { return TLSBaseAddr; }

  /// RevLoader: Gets TLS size
  const uint64_t& GetTLSSize() { return TLSSize; }

  // friend std::ostream& operator<<(std::ostream &os, const Elf64_Ehdr &header){ };

private:
  std::string exe;          ///< RevLoader: binary executable
  std::string args;         ///< RevLoader: program args
  RevMem *mem;              ///< RevLoader: memory object
  SST::Output *output;      ///< RevLoader: output handler

  uint32_t RV32Entry;       ///< RevLoader: RV32 entry
  uint64_t RV64Entry;       ///< RevLoader: RV64 entry

  uint64_t TLSBaseAddr = 0;
  uint64_t TLSSize = 0;

  ElfInfo elfinfo;          ///< RevLoader: elf info from the loaded program

  std::map<std::string, uint64_t> symtable;       ///< RevLoader: loaded symbol table
  std::map<uint64_t,std::string> tracer_symbols;  ///< RevLoader: address to symbol for tracer

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
  void splitStr(const std::string& s, char c, std::vector<std::string>& v);

  ///< Breaks bulk writes into cache lines
  bool WriteCacheLine(uint64_t Addr, size_t Len, void *Data);

  ///< RevLoader: Replaces first MemSegment (initialized to entire memory space) with the static memory
  void InitStaticMem();

}; // class Loader

} // namespace SST::RevCPU

#endif

// EOF
