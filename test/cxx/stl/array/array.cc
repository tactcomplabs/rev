//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <array>
#include <stdexcept>

#include "rev-macros.h"
#include "revalloc.h"

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

#define DIM 450  // Large array test
#define N   10

typedef std::basic_string<char, std::char_traits<char>, Allocator<char>> revString;

void testAlloc() {
  std::array<int, DIM> arr;

  for( int i = 0; i < DIM; i++ ) {
    arr[i] = i;
  }

  int expected_sum = DIM * ( DIM - 1 ) / 2;
  int sum          = 0;
  for( int i = 0; i < DIM; i++ ) {
    sum += arr[i];
  }

  assert( sum == expected_sum );
}

void testBoundsCheck() {
  std::array<int, DIM> arr;

  try {
    arr.at( DIM ) = DIM;  // accessing DIM using at should throw an exception
    assert( false );
  } catch( const std::out_of_range& e ) {
    // success
  }
}

int main() {

  //When constructing the map the correct method is to use the default constructor and then
  //  insert each element as shown (using std::pair
  std::array<float, DIM> arr;

  testAlloc();
  testBoundsCheck();

  return 0;
}
