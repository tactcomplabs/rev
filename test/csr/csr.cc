/*
 * csr.cc
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "syscalls.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define assert( x )      \
  do {                   \
    if( !x )             \
      asm( " .word 0" ); \
  } while( 0 )

enum class CSROp { Write, Set, Clear };

enum class OpKind { Imm, Reg };

template<CSROp OP, OpKind KIND, uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst;

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Write, OpKind::Reg, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrw zero, %0, zero" ::"I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrw zero, %0, %1" ::"I"( CSR ), "r"( arg ) );
      }
    } else {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrw %0, %1, zero" : "=r"( old ) : "I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrw %0, %1, %2" : "=r"( old ) : "I"( CSR ), "r"( arg ) );
      }
    }
    return old;
  }
};

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Set, OpKind::Reg, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrs zero, %0, zero" ::"I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrs zero, %0, %1" ::"I"( CSR ), "r"( arg ) );
      }
      return old;
    } else {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrs %0, %1, zero" : "=r"( old ) : "I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrs %0, %1, %2" : "=r"( old ) : "I"( CSR ), "r"( arg ) );
      }
      return old;
    }
  }
};

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Clear, OpKind::Reg, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrc zero, %0, zero" ::"I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrc zero, %0, %1" ::"I"( CSR ), "r"( arg ) );
      }
    } else {
      if constexpr( ARG == 0 ) {
        asm volatile( "csrrc %0, %1, zero" : "=r"( old ) : "I"( CSR ) );
      } else {
        size_t arg = ARG;
        asm volatile( "csrrc %0, %1, %2" : "=r"( old ) : "I"( CSR ), "r"( arg ) );
      }
    }
    return old;
  }
};

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Write, OpKind::Imm, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      asm volatile( "csrrwi zero, %0, %1" ::"I"( CSR ), "K"( ARG ) );
    } else {
      asm volatile( "csrrwi %0, %1, %2" : "=r"( old ) : "I"( CSR ), "K"( ARG ) );
    }
    return old;
  }
};

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Set, OpKind::Imm, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      asm volatile( "csrrsi zero, %0, %1" ::"I"( CSR ), "K"( ARG ) );
    } else {
      asm volatile( "csrrsi %0, %1, %2" : "=r"( old ) : "I"( CSR ), "K"( ARG ) );
    }
    return old;
  }
};

template<uint16_t CSR, size_t ARG, bool RD0>
struct CSRInst<CSROp::Clear, OpKind::Imm, CSR, ARG, RD0> {
  size_t operator()() {
    size_t old = 0;
    if constexpr( RD0 ) {
      asm volatile( "csrrci zero, %0, %1" ::"I"( CSR ), "K"( ARG ) );
    } else {
      asm volatile( "csrrci %0, %1, %2" : "=r"( old ) : "I"( CSR ), "K"( ARG ) );
    }
    return old;
  }
};

template<uint16_t CSR>
size_t ReadCSR() {
  size_t old;
  asm volatile( "csrr %0, %1" : "=r"( old ) : "I"( CSR ) );
  return old;
}

size_t testnum = 0;

template<CSROp OP, OpKind KIND, uint16_t CSR, size_t ARG, size_t INIT, bool RD0>
void CSRTest1() {
  ++testnum;

#if 0
  char test[128];
  snprintf( test, sizeof( test ), " Test %zu:\n", testnum );
  rev_write( 1, test, strlen( test ) );
#endif

  // Initialize the CSR register to INIT
  CSRInst<CSROp::Write, KIND, CSR, INIT, true>{}();

  // Perform the CSR operation with the argument
  size_t oldval = CSRInst<OP, KIND, CSR, ARG, RD0>{}();

  // Read the new value of the CSR
  size_t newval = ReadCSR<CSR>();

  // Old value must be initial value or zero
  if constexpr( RD0 ) {
    assert( oldval == 0 );
  } else {
    assert( oldval == INIT );
  }

  // New value according to operation
  switch( OP ) {
  case CSROp::Write: assert( newval == ARG ); break;
  case CSROp::Set: assert( newval == ( INIT | ARG ) ); break;
  case CSROp::Clear: assert( newval == ( INIT & ~ARG ) ); break;
  }
}

template<OpKind KIND, uint16_t CSR, size_t ARG, size_t INIT, bool RD0>
void CSRTest2() {
  CSRTest1<CSROp::Write, KIND, CSR, ARG, INIT, RD0>();
  CSRTest1<CSROp::Set, KIND, CSR, ARG, INIT, RD0>();
  CSRTest1<CSROp::Clear, KIND, CSR, ARG, INIT, RD0>();
}

template<OpKind KIND, uint16_t CSR, size_t INIT, bool RD0>
void CSRTest3() {
  CSRTest2<KIND, CSR, +size_t{ 0 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 1 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 2 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 3 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 4 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 8 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 16 }, INIT, RD0>();
  CSRTest2<KIND, CSR, +size_t{ 31 }, INIT, RD0>();

  if constexpr( KIND == OpKind::Reg ) {
    CSRTest2<KIND, CSR, ~size_t{ 0 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 1 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 2 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 3 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 4 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 8 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 16 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 31 }, INIT, RD0>();
    CSRTest2<KIND, CSR, +size_t{ 256 }, INIT, RD0>();
    CSRTest2<KIND, CSR, ~size_t{ 256 }, INIT, RD0>();
    CSRTest2<KIND, CSR, +( size_t{ 1 } << sizeof( size_t ) * 8 - 1 ), INIT, RD0>();
    CSRTest2<KIND, CSR, ~( size_t{ 1 } << sizeof( size_t ) * 8 - 1 ), INIT, RD0>();
  }
}

template<OpKind KIND, uint16_t CSR, bool RD0>
void CSRTest4() {
  CSRTest3<KIND, CSR, +size_t{ 0 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 1 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 2 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 3 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 4 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 8 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 16 }, RD0>();
  CSRTest3<KIND, CSR, +size_t{ 31 }, RD0>();

  if constexpr( KIND == OpKind::Reg ) {
    CSRTest3<KIND, CSR, ~size_t{ 0 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 1 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 2 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 3 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 4 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 8 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 16 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 31 }, RD0>();
    CSRTest3<KIND, CSR, +size_t{ 256 }, RD0>();
    CSRTest3<KIND, CSR, ~size_t{ 256 }, RD0>();
    CSRTest3<KIND, CSR, +( size_t{ 1 } << sizeof( size_t ) * 8 - 1 ), RD0>();
    CSRTest3<KIND, CSR, ~( size_t{ 1 } << sizeof( size_t ) * 8 - 1 ), RD0>();
  }
}

template<uint16_t CSR>
void CSRTest5() {
  CSRTest4<OpKind::Imm, CSR, false>();
  CSRTest4<OpKind::Reg, CSR, false>();
  CSRTest4<OpKind::Imm, CSR, true>();
  CSRTest4<OpKind::Reg, CSR, true>();
}

int main( int argc, char** argv ) {

  CSRTest5<1>();     // fflags
  CSRTest5<2>();     // frm
  CSRTest5<3>();     // fcsr
  CSRTest5<0xff>();  // A random unprivileged read-write CSR
  return 0;
}
