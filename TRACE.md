Overview
--------

This version of RevTracer represents a proof-of-concept
prototype for evaluation purposes only.

This code lives on the 'tracer' branch and should never
be released on any publicly supported branches.

Usage
-----

The tracer will be enabled for verbosity >= 5.

Testing
-------

After building following the instructions in Readme.md
the tracer can be tested as follows:

  cd test/tracer; ./run_tracer.sh > trc; diff gold.trc trc


Issues
------

- Need to be able to link to the spike disasm.a file.
  For the prototype ALL the includes and src were copied over
  which pollutes the code base.

- Need to wrap the RegFile class in a way that allows selective
  tracing. Macros are used in the various RV*.h files to
  capture data for tracing.

- Tracing for register and memory access in RV32I.h and RV64I.h
  is enabled. All other RV*.h files will not be traced. Fencing,
  ECalls, and any other operations that are not simple memory or
  register operations are also not traced.

