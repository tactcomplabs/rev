//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "rev-macros.h"
#include "revalloc.h"
#include <vector>

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

int main() {

  class TestObj {
  public:
    TestObj( int m1, int m2 ) {
      mem1 = m1;
      mem2 = m2;
    }

    TestObj() {
      mem1 = 0;
      mem2 = 0;
    }

    ~TestObj() {};

    int GetM1() { return mem1; }

    int GetM2() { return mem2; }

    void SetM1( int m ) { mem1 = m; }

    void SetM2( int m ) { mem2 = m; }

  private:
    int mem1;
    int mem2;
  };

  std::vector<TestObj, Allocator<TestObj>> v;

  TestObj c;
  v.push_back( c );
  v[0].SetM1( 0xbeef );
  assert( v[0].GetM1() == 0xbeef )

    return 0;
}
