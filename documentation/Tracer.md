1. Overview
------------

Provides compact visualization of instruction execution with disassembly,
register, and core initiated memory access shown on a single line.
register accesses shown on one line.

2. Usage

2.1 Compile options
-------------------

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

2.2 Runtime options
-------------------
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

3. Test Sample

3.1 Build and Run
-------------------

After building using the instructions in Readme.me:

   cd test/tracer; ./run_tracer.sh 

3.2. Tracing Macros
-------------------

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

// tracing starts
RevCPU[cpu0:InstTrace:18156000]: Core 0; Hart 0; Thread 1]; *I 0x10330:00301013  + slli    zero, zero, 3	 0x0<-zero zero<-0x0 pc<-0x10334 
RevCPU[cpu0:InstTrace:18157000]: Core 0; Hart 0; Thread 1]; *I 0x10334:fec42703    lw      a4, -20(s0)	 0x3ffffba0<-s0 0x000003de<-[0x3ffffb8c,4] a4<-0x3de 
RevCPU[cpu0:InstTrace:18158000]: Core 0; Hart 0; Thread 1]; *I 0x10338:3de00793    li      a5, 990	 0x0<-zero a5<-0x3de 
RevCPU[cpu0:InstTrace:18159000]: Core 0; Hart 0; Thread 1]; *I 0x1033c:00f70463    beq     a4, a5, pc + 8	 0x3de<-a4 0x3de<-a5 pc<-0x10344 
RevCPU[cpu0:InstTrace:18160000]: Core 0; Hart 0; Thread 1]; *I 0x10344:00201013  - slli    zero, zero, 2	 0x0<-zero zero<-0x0
// no tracing between cycles 18160 and 27090
RevCPU[cpu0:InstTrace:27090000]: Core 0; Hart 0; Thread 1]; *I 0x10358:00301013  + slli    zero, zero, 3	 0x0<-zero zero<-0x0 pc<-0x1035c 
RevCPU[cpu0:InstTrace:27091000]: Core 0; Hart 0; Thread 1]; *I 0x1035c:fec42783    lw      a5, -20(s0)	 0x3ffffba0<-s0 0x00000000<-[0x3ffffb8c,4] a5<-0x0 
RevCPU[cpu0:InstTrace:27092000]: Core 0; Hart 0; Thread 1]; *I 0x10360:00078463    beqz    a5, pc + 8	 0x0<-a5 0x0<-zero pc<-0x10368 
RevCPU[cpu0:InstTrace:27093000]: Core 0; Hart 0; Thread 1]; *I 0x10368:00201013  - slli    zero, zero, 2	 0x0<-zero zero<-0x0 
// tracing ends

4.  Tracer Output
-------------------

4.1 Logging prefix
-------------------

  The tracer output is prefixed by the standard logging information with
  an additional text to support extraction from the log file.

   |--   Standard logging prefix                           --| key |
    RevCPU[cpu0:InstTrace:18156000]: Core 0; Hart 0; Thread 1]; *I 

4.2 Instruction Disassembly
--------------------------

4.2.1 REV_USE_SPIKE=ON
----------------------

  |  PC  : INSN      |  Disassembly     |
  0x10474:00c50533    add     a0, a0, a2

4.2.2 REV_USE_SPIKE=OFF
-----------------------

  |  PC  : INSN      |  Disassembly      |
  0x10474:00c50533    add %rd, %rs1, %rs2

4.3 Register to Register Format
--------------------------------

  |  PC   : INSN     |  Disassembly             |         Effects          |
   0x10474:00c50533    add     a0, a0, a2	 0x14<-a0 0x50<-a2 a0<-0x64 

  Notice that the effects include the same register naming conventions
  as the disassembled instruction. 

  Effects interpretation:
   data<-reg  : Source register read
   reg<-data  : Destination register write

4.4 Register to Memory (Store)
------------------------------
  |  PC  : INSN    |     Disassembly    |
  0x10220:fef42623  sw      a5, -20(s0)	 

  |                    Effects                        |
    0x3ffffb60<-s0 0x64<-a5 [0x3ffffb4c,4]<-0x00000064

  Effects interpretation:
   data<-reg  : Source register read
   [logical address, number of bytes]<-data : write initiated from core

4.5 Memory to Register (Load)
-----------------------------

  |  PC  : INSN    |    Disassembly      |
  0x102d4:fec42783    lw      a5, -20(s0)	 

  |                          Effects                              |
  0x3fefff90<-s0 0x0<-a5 0xacee1190<-[0x3fefff7c,4] a5<-0xacee1190  
  
  Effects interpretation:

   data<-reg  : Source register read
   reg<-data  : Destination register write
   reg<-[logical address, number of bytes] : returning load data
   
4.6 Program Counter Writes (Branches/Jumps)
--------------------------------------------

  |  PC   | INSN   |  Disassembly  |             Effects                       |
  0x102a8:f35ff0ef  jal  pc - 0xcc  0x0<-ra ra<-0x102ac pc<-0x101dc <_Z5checki>

  Effects interpretation:
   data<-reg  : Source register read
   reg<-data  : Destination link register write
   pc<-address: Resolved branch target
   <_Z5checki> : Matching ELF symbol associated with new PC
   

5. Issues/Enhancements
----------------------

  - Instruction tracing for integer operations is mostly covered. Other
    instruction types (floating point, coprocessor, vector,... ) are not.
  
  - ECalls are also not yet traced

  - Tracing information may be sent to different output streams in more
    compact formats in the future.

  - When using the internal Rev instruction formatter only the opcode
    is printed. It should be possible to support a full disassembler
    within REV.

