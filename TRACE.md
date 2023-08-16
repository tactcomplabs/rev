1. Overview
-----------

This version of RevTracer represents a proof-of-concept
prototype for evaluation purposes only.

This code lives on the 'tracer' branch and should never
be released on any publicly supported branches.

2. Usage
--------

The tracer will be enabled for verbosity >= 5.

3. Testing
----------

After building following the instructions in Readme.md
the tracer can be tested as follows:

  cd test/tracer; ./run_tracer.sh > trc; diff gold.trc trc


4. Tracer Output
----------------

4.1 Logging prefix
  The tracer output is prefixed by the standard logging information with
  an additional text to support extraction from the log file.

   |--   Standard logging prefix               --| key |
   RevCPU[cpu:ClockTick:5000]: Core 7 ; Thread 0]; *I

  This prefix will be omitted for the examples in this document.

4.2 Register to Register Format

  |  PC   | INSN   |  Disassembly   |              Effects                   |
   0x10220:03010413 addi s0, sp, 48  0x3fefff60<-sp 0x11f50<-s0 s0<-0x3fefff90


  Notice that the effects include the same register naming conventions
  as the disassembled instruction. 

  Effects interpretation:
   0x3fefff60<-sp  : Source register read
   0x11f50<-s0     : Original value of destination register. This is not an
                     actual read but it is provided for debugging convenience.
   s0<-0x3fefff90  : Destination register write

4.3 Register to Memory (Store)

  |  PC   | INSN    | Disassembly  |
   0x101e0:00812e23   sw s0, 28(sp) 

  |                      Effects                          |
  0x3fefff40<-sp 0x3fefff90<-s0 [0x3fefff5c,4]<-0x3fefff90 

  Effects interpretation:
   0x3fefff40<-sp  : Source register read
   0x3fefff90<-s0  : Source register read
   [0x3fefff5c,4]<-0x3fefff90 : [Logical Mem Address, Len(bytes)] <- Data

4.4 Memory to Register (Load)

  |  PC   | INSN    |    Disassembly    |
  0x102d4:fec42783    lw      a5, -20(s0)	 

  |                          Effects                              |
  0x3fefff90<-s0 0x0<-a5 0xacee1190<-[0x3fefff7c,4] a5<-0xacee1190  
  
  Effects interpretation:

    0x3fefff90<-s0  : Source register read
    0x0<-a5         : Destination register original value (convenience)
    0xacee1190<-[0x3fefff7c,4] : Data <- [Logical Mem Address, Len(bytes)]
    a5<-0xacee1190  : Register write
    
4.5 Program Counter Writes (Branches/Jumps)

  |  PC   | INSN   |  Disassembl y  |             Effects            |
   0x102a8:f35ff0ef  jal  pc - 0xcc   0x0<-ra ra<-0x102ac pc<-0x101dc 

  Effects interpretation:
   0x0<-ra     : Destinatino register read (convenience)
   ra<-0x102ac : Destintation link register write
   pc<-0x101dc : Resolved branch target

5. Programmer controls
---------------------

 Currently there is a single control that can be used to disable and
 enable tracing in assembly code on a per core basis.

   asm volatile("xor x0,x0,x0");  // 0x4033 toggle tracing enable

 This is particularly useful in avoiding tracing polling loops that
 could generate millions of lines of useless tracing information.

 In the trace, the trace on/off information is rendered in a special
 'event' field as shown below.

                                  * Event field ('-' turning trace off )
 RevCPU[cpu:ClockTick:14000]: ... - xor zero, zero, zero  ...
                                        * Event field ('-' turning trace on )
 RevCPU[cpu:ClockTick:1100022000]: ...  + xor     zero, zero, zero ...

                                                        Event Field ->|
 RevCPU[cpu:ClockTick:14000]: Core 7; Thread 0]; *I 0x10244:00004033  - \
      xor     zero, zero, zero	 0x0<-zero 0x0<-zero 0x0<-zero zero<-0x0 
 RevCPU[cpu:ClockTick:1100022000]: Core 0 ... ]; *I 0x1027c:00004033  + \
      xor     zero, zero, zero	 0x0<-zero 0x0<-zero 0x0<-zero zero<-0x0 

 The '-' event indicates the tracing is disabled.
 The '+' event indicates the tracing is enbabled.

 Observe the large number of ClockTick's in the intervening code that
 was used by a polling loop.

6. Issues/Enhancements
----------------------

  - Threading has not been tested. Conversely, testing has only
    been done using
          #define _REV_HART_COUNT_ 1 (RevInstTable.h)

  - Network requests are not currently traced. For this initial
    prototype only the execution phase of an instruction is traced.
  
  - Tracing for register and memory access in RV32I.h and RV64I.h
    is enabled. All other RV*.h files will not be traced. Fencing,
    ECalls, and any other operations that are not simple memory or
    register operations are also not traced.

  - Need to be able to link to the spike disasm.a file.
    For the prototype ALL the includes and src were copied over
    which pollutes the code base.

  - Would prefer to support disassembler selected at run-time based on
    what is available in the toolchain.

  - Adding symbols to the trace would significantly improve the debugging
    productivity. This, ideally, would be provided in the disassembler.

  - Would prefer to wrap the RegFile class in a way that allows selective
    tracing. Macros are used in the various RV*.h files to capture data.
    If these operations can be encapsulated in the RegFile class (most of)
    the macros can be removed.

  - The trace output is simple sent out as log messages. It would
    be advantageous to allow streaming to other files to allow
    binary or json output in addition to the text.

