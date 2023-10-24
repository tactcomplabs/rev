# Overview

The Rev tracer currently provides compact visualization of instruction
execution with disassembly and effects on registers and memory. The
architecture is intended to be extensible to support more complex pipelines
and controls to enhance productivity while reducing simulator output and
associated post-processing.

# Usage

## Compile options

The execution tracer is configured using the following cmake compile options:

 REV_TRACER
	ON : compile for tracing (default)
	OFF: do not compile in tracing support 
	
 REV_USE_SPIKE :
 	ON  : Use spike libdiasm.a for disassembly
	OFF : Use internal REV instruction format (default)
        
When setting REV_USE_SPIKE to ON, GCC tools and links to libdisasm.a
must be available. These are part of rev_isa_sim (Spike). If this library is
not found the tracer should revert to the an internal REV format.

Previous instruction execution trace output is replaced by the compact tracer
format. Compiling with REV_TRACER=0 will revert the output to to the old
format and elliminate any possible performance impacts.

## Runtime options

The tracer is enabled using verbosity >= 5.

sst-info revcpu will now includes the following additional options:

  trcOp: Tracer instruction trigger  [slli]
  trcLimit: Max trace lines per core (0 no limit)  [0]
  trcStartCycle: Starting tracer cycle (disables trcOp)  [0]

To achieve compact traces, the tracer is initially off. The user must
enable it using one of two methods:

  a. Programatic Controls: These are 'nop' instructions embedded in the
     target code that will activate or deactive tracing. This allows for
     tracing only specific code of interest and avoiding extreme file sizes.
     The 'trcOp' option provides the base opcode and must be the following
     subset of custom hints defined in the RISCV specification:
     
     	    slti, sltiu, slli, srli, srai


  b. Cycle Based control: This simply allows the user to specify
     a starting cycle to start tracing for a CPU. This will affect
     affect all cores and will defeat all programatic controls.
     However, setting 'trcCycle' to 0 will have no effect.

     Important: trcStartCycle and trcLimit are specified in terms
     of REV cycles, not time.

## Test Sample

## Build and Run

After building the Rev core  using the instructions provided in Readme.md,
simply

   cd test/tracer; make clean; make

Excerpts of source code and associated output are provided in the sample
traces at the end of this document.

## Tracing Macros

Programatic controls are realized through macros provided in 'rev-macros.h'
'. These are by default

   #define TRACE_OFF      asm volatile("slli x0,x0,0"); 
   #define TRACE_ON       asm volatile("slli x0,x0,1"); 
   #define TRACE_PUSH_OFF asm volatile("slli x0,x0,2"); 
   #define TRACE_PUSH_ON  asm volatile("slli x0,x0,3"); 
   #define TRACE_POP      asm volatile("slli x0,x0,4");

If there is a conflict with the 'slli' instruction, it can instruction
can be changed using the 'trcOp' option. The header file must be changed
to match or tracing will not be enabled. See 2.2 for the list of
supported instructions.

When one of these instruction is encountered the trace will indicate
and on or off event using '+' or '-' before the instruction mnemonic.

Example: 

The prefix fields showing cycle, cpu, thread, and hart information are omitted.

// Tracing starts
0x10330:00301013  + slli    zero, zero, 3	 0x0<-zero zero<-0x0 pc<-0x10334
0x10334:fec42703    lw      a4, -20(s0)	 	 0x3ffffba0<-s0 0x000003de<-[0x3ffffb8c,4] a4<-0x3de 
0x10338:3de00793    li      a5, 990	 	 0x0<-zero a5<-0x3de 
0x1033c:00f70463    beq     a4, a5, pc + 8	 0x3de<-a4 0x3de<-a5 pc<-0x10344
0x10344:00201013  - slli    zero, zero, 2	 0x0<-zero zero<-0x0
// Tracing ends and then resumes at a later cycle
0x10358:00301013  + slli    zero, zero, 3	 0x0<-zero zero<-0x0 pc<-0x1035c 
0x1035c:fec42783    lw      a5, -20(s0)	 	 0x3ffffba0<-s0 0x00000000<-[0x3ffffb8c,4] a5<-0x0 
0x10360:00078463    beqz    a5, pc + 8	 	 0x0<-a5 0x0<-zero pc<-0x10368 
0x10368:00201013  - slli    zero, zero, 2	 0x0<-zero zero<-0x0 
// tracing ends

# Tracer Output

## Logging prefix

  The tracer output is prefixed by the standard logging information with
  an additional text to support extraction from the log file.

   |--   Standard logging prefix                           --| key |
    RevCPU[cpu0:InstTrace:18156000]: Core 0; Hart 0; Thread 1]; *I 

## Instruction Disassembly

### REV_USE_SPIKE=ON

  |  PC  : INSN      |  Disassembly     |
  0x10474:00c50533    add     a0, a0, a2

### REV_USE_SPIKE=OFF

  |  PC  : INSN      |  Disassembly      |
  0x10474:00c50533    add %rd, %rs1, %rs2

## Register to Register Format

  |  PC   : INSN     |  Disassembly             |         Effects          |
   0x10474:00c50533    add     a0, a0, a2	 0x14<-a0 0x50<-a2 a0<-0x64 

  Notice that the effects include the same register naming conventions
  as the disassembled instruction. 

  Effects interpretation:
   data<-reg  : Source register read
   reg<-data  : Destination register write

## Register to Memory (Store)

  |  PC  : INSN    |     Disassembly    |
  0x10220:fef42623  sw      a5, -20(s0)	 

  |                    Effects                        |
    0x3ffffb60<-s0 0x64<-a5 [0x3ffffb4c,4]<-0x00000064

  Effects interpretation:
   data<-reg  : Source register read
   [logical address, number of bytes]<-data : write initiated from core

## Memory to Register (Load)

  |  PC  : INSN    |    Disassembly      |
  0x102d4:fec42783    lw      a5, -20(s0)	 

  |                          Effects                              |
  0x3fefff90<-s0 0x0<-a5 0xacee1190<-[0x3fefff7c,4] a5<-0xacee1190  
  
  Effects interpretation:

   data<-reg  : Source register read
   reg<-data  : Destination register write
   reg<-[logical address, number of bytes] : returning load data
   
## Program Counter Writes (Branches/Jumps)

  |  PC   | INSN   |  Disassembly  |             Effects                       |
  0x102a8:f35ff0ef  jal  pc - 0xcc  0x0<-ra ra<-0x102ac pc<-0x101dc <_Z5checki>

  Effects interpretation:
   data<-reg  : Source register read
   reg<-data  : Destination link register write
   pc<-address: Resolved branch target
   <_Z5checki> : Matching ELF symbol associated with new PC
   

# Issues/Enhancements

  - Instruction tracing for integer operations is mostly covered. Other
    instruction types (floating point, coprocessor, vector,... ) are not.
  
  - ECalls are also not yet traced

  - Tracing information may be sent to different output streams in more
    compact formats in the future.

  - When using the internal Rev instruction formatter only the opcode
    is printed. It should be possible to support a full disassembler
    within REV.In general, threading with tracing has not been tested.
    

# Sample Traces

The source code for these samples is found in <rev>/test/tracer.
All the trace prefixes have been removed or modified for readability.

Review tracer.c and the generated disassembly and log files for additional
illustrative trace test cases.

## Traced assertion macro

  This example demonstrates the usage of the following macros defined
  in rev-macros.h 
  
  #define TRACE_PUSH_ON  asm volatile("slli x0,x0,3"); 
  #define TRACE_POP      asm volatile("slli x0,x0,4");
  #define TRACE_ASSERT(x) { TRACE_PUSH_ON; \
  if (!(x)) { asm volatile(".word 0x0"); }; \
  TRACE_PUSH_OFF }

  For this line of C code:
  TRACE_ASSERT(res*2==1980);

  We have the following disassembly.

  147e4:	00301013          	sll	zero,zero,0x3  // TRACE_PUSH_ON
  147e8:	fec42703          	lw	a4,-20(s0)
  147ec:	3de00793          	li	a5,990
  147f0:	00f70463          	beq	a4,a5,147f8 <main+0xa4>
  147f4:	0000                	.2byte	0x0
  147f6:	0000                	.2byte	0x0
  147f8:	00201013          	sll	zero,zero,0x2  // TRACE_POP

  And the following trace:

  1  0x147e4:00301013  + slli  zero, zero, 3   0x0<-zero zero<-0x0 pc<-0x147e8 
  2  0x147e8:fec42703    lw      a4, -20(s0)   0x3ffffba0<-s0 \
					       0x000003de<-[0x3ffffb8c,4] \
					       a4<-0x3de 
  3  0x147ec:3de00793    li      a5, 990       0x0<-zero a5<-0x3de 
  4  0x147f0:00f70463    beq     a4, a5, pc+8  0x3de<-a4 0x3de<-a5 pc<-0x147f8 
  5  0x147f8:00201013  - slli    zero, zero, 2  0x0<-zero zero<-0x0 

 Line by line interpretation:
 
 Cycle 1: The '+' indicates the first line of a trace sequence. This
          TRACE_PUSH_ON macro will push the current state of the tracer
	  onto a stack and enable tracing.
	  
          0x0<-zero (twice) : indicates the x0 register has been read twice
	  pc<-0x147e8: The tracer prints the program counter at the start
	               of new traces or for any branch occurance.
		       
 Cycle 2: 0x3ffffba0<-s0 : s0 read for part of the memory address calculation.
          0x000003de<-[0x3ffffb8c,4]:
	  4 bytes are read from memory location 0x3ffffb8c. The data is 0x3de.
          a4<-0x3de : Register a4 is written with the return load data.

 Cycle 3: 0x0<-zero:  'li' is  implementation an 'addi' using the x0 register.
       	  a5<-0x3de:  a5 is written with the immediate field

 Cycle 4: 0x3de<-a4 0x3de<-a5: Read of the 2 sources being compared.
          pc<-0x147f8: target instruction address for the taken branch.
 
 Cycle 5: The '-' indicates that tracing is being disabled.

## 32-bit vs 64-bit instructions

   Here is a sequence of memory accesses using byte, half-word, and word data.
   The data is -1 in order to observe the effects of sign extensions.
   
   This sequence is run on using REV configured for RV32I instructions.

   li      t3, -1	 0x0<-zero t3<-0xffffffff
   sb      t3, 0(a5)	 0x3ffffb78<-a5 0xff<-t3 [0x3ffffb78,1]<-0xff 
   lb      t4, 0(a5)	 0x3ffffb78<-a5 0xff<-[0x3ffffb78,1] t4<-0xffffffff 
   sh      t3, 0(a5)	 0x3ffffb78<-a5 0xffff<-t3 [0x3ffffb78,2]<-0xffff 
   lh      t4, 0(a5)	 0x3ffffb78<-a5 0xffff<-[0x3ffffb78,2] t4<-0xffffffff
   sw      t3, 0(a5)	 0x3ffffb78<-a5 0xffffffff<-t3 \
                         [0x3ffffb78,4]<-0xffffffff 
   lw      t4, 0(a5)	 0x3ffffb78<-a5 0xffffffff<-[0x3ffffb78,4] \
                         t4<-0xffffffff

   This next sequence also includes 64-bit operations and is run using RV64G
   instructions.
   
   li      t3, -1	 0x0<-zero t3<-0xffffffffffffffff 
   sb      t3, 0(a5)	 0x3ffffb28<-a5 0xff<-t3 [0x3ffffb28,1]<-0xff 
   lb      t4, 0(a5)	 0x3ffffb28<-a5 0xff<-[0x3ffffb28,1]
                         t4<-0xffffffffffffffff 
   sh      t3, 0(a5)	 0x3ffffb28<-a5 0xffff<-t3 [0x3ffffb28,2]<-0xffff 
   lh      t4, 0(a5)	 0x3ffffb28<-a5 0xffff<-[0x3ffffb28,2]
                         t4<-0xffffffffffffffff 
   sw      t3, 0(a5)	 0x3ffffb28<-a5 0xffffffff<-t3 \
                         [0x3ffffb28,4]<-0xffffffff 
   lw      t4, 0(a5)	 0x3ffffb28<-a5 0xffffffff<-[0x3ffffb28,4]
                         t4<-0xffffffffffffffff 
   addi    a5, s0, -48	 0x3ffffb58<-s0 a5<-0x3ffffb28 
   sd      t3, 0(a5)	 0x3ffffb28<-a5 0xffffffffffffffff<-t3
   	       		 [0x3ffffb28,8]<-0xffffffffffffffff 
   lwu     t4, 0(a5)	 0x3ffffb28<-a5 0xffffffff<-[0x3ffffb28,4]
                         t4<-0xffffffff 
   ld      t4, 0(a5)	 0x3ffffb28<-a5 0xffffffffffffffff<-[0x3ffffb28,8]
                         t4<-0xffffffffffffffff 

   Observe that the data for each memory read matches the data size.
   
   sb:      [0x3ffffb28,1]<-0xff 		    
   lb:                0xff<-[0x3ffffb28,1]
   sh:      [0x3ffffb28,2]<-0xffff 	 
   lh:              0xffff<-[0x3ffffb28,2]
   sw:      [0x3ffffb28,4]<-0xffffffff   
   lw:          0xffffffff<-[0x3ffffb28,4]
   sd:      [0x3ffffb28,8]<-0xffffffffffffffff
   lwu:         0xffffffff<-[0x3ffffb28,4] 
   ld:  0xffffffffffffffff<-[0x3ffffb28,8]

   The final register writes for loads are sign-extended except when using lwu.
   The register write size depends on whether using 32-bit or 64-bit designs.

   lb (32-bit): t4<-0xffffffff 
   lb (64-bit): t4<-0xffffffffffffffff 
   lwu(64-bit): t4<-0xffffffff 
   ld (64-bit): t4<-0xffffffffffffffff 
