# Co-processors

A `RevCoProc` is a SST subcomponent (slot `co_proc` of RevCPU) attached to one core. This page
covers how the core hands it work, and the base class for a co-processor with a memory port of
its own.

## Taking instructions

There are two ways, and a co-processor may use either.

* **`IssueInst(F, R, M, Inst)`**: the core offers every instruction it cannot decode. If the
  co-processor claims it, the core replaces it by a NOP and runs on. The registers the
  instruction names have not been checked for hazards, and the co-processor must read them and
  stall the hart (`ExternalStallHart`) itself if it needs them stable.
* **`Command(cmd, result)`** with the `Xcoproc` extension (`RV64GC_Xcoproc` in the `machine`
  string): custom-0..3 are decoded by the core as R-type instructions in the encoding RoCC uses,
  `funct7` the command and `funct3` = `{xd, xs1, xs2}`. The core reads the source registers the
  flags name, calls `Command()` with their values (`RevCoProcCmd`), and writes `rd` with `result`
  if `xd` is set, so its hazard checks apply. The answer is taken when the instruction executes.
  `Command()` returns false for a command it does not handle, which stops the simulation. The
  extension is in `include/insns/Xcoproc.h`; what a command means is up to the co-processor.

## A co-processor with its own memory port: `RevMemCoProc`

A co-processor that reads and writes memory (an accelerator working on the program's arrays) has
a StandardMem interface of its own, which the configuration connects to the same cache or bus as
the core's, so it uses the program's addresses and sees what the core has written. `RevMemCoProc`
(`include/RevMemCoProc.h`) does the parts every such co-processor needs:

| It does | You do |
|---|---|
| loads the memory interface from a subcomponent slot (default `memory`) | document the slot in your `SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS` |
| gives it, as time base, a clock of its own (the frequency you pass to the constructor) | implement `MemClock(cycle)` for that clock |
| routes the interface's responses to `HandleMemory(req)` | implement it |
| forwards `init`, `setup`, `complete` and `finish` to the interface | call the base class's version first when you override them |

```cpp
class MyAccel final : public RevMemCoProc {
public:
  SST_ELI_REGISTER_SUBCOMPONENT( MyAccel, "mylib", "MyAccel", SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                                 "an accelerator", SST::RevCPU::RevCoProc )
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS( { "memory", "Memory interface", "SST::Interfaces::StandardMem" } )

  MyAccel( ComponentId_t id, Params& params, RevCore* parent )
    : RevMemCoProc( id, params, parent, params.find<std::string>( "clock", "1GHz" ) ) {}

  bool Command( const RevCoProcCmd& cmd, uint64_t& result ) override { /* ... */ return true; }
  bool IssueInst( const RevFeature*, RevRegFile*, RevMem*, uint32_t ) override { return false; }
  bool Reset() override { return true; }
  bool Teardown() override { return true; }
  bool ClockTick( SST::Cycle_t ) override { return true; }   // once per core cycle
  bool IsDone() override { return true; }
protected:
  void HandleMemory( StdMem::Request* req ) override { /* a response to one of your requests */ }
  bool MemClock( SST::Cycle_t ) override { return false; }    // the co-processor's own clock
};
```

In the Python configuration the co-processor's interface is a `memHierarchy.standardInterface`
whose `lowlink` goes to the same bus or cache as the core's (see the bambu example in
`soda-sst-system`, `configs/bambu_rev.py`).

## Things to know

* SST calls `init`, `setup`, `complete` and `finish` only on top-level components. RevCPU forwards
  them to its co-processor (`RevCPU::init` and friends), so a memory interface in a co-processor
  takes part in the init phases and untimed data reaches it.
* `RevCoProc::ClockTick` is called by RevCPU once per core cycle and is the place to work the
  instruction queue; `MemClock` is the co-processor's own clock, at its own frequency.
* A core has one co-processor (`RevCore::SetCoProc` is fatal on a second). A co-processor that
  serves several devices routes by custom opcode itself.
* Rev passes the program's virtual addresses straight to memHierarchy, so an accelerator's
  pointers are the program's pointers.
