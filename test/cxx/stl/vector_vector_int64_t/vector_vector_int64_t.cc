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

#include <cstdint>
#include <utility>  // For std::pair

#include <cstdio>
#include <stdexcept>

#define assert( x )      \
  if( !( x ) ) {         \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
    asm( ".byte 0x00" ); \
  }

#define N 20
#define M 40

bool testSize() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec = {
    {1, 2},
    {3, 4}
  };
  if( vec.size() != 2 )
    return false;

  vec.push_back( { 5, 6, 7 } );
  return vec.size() == 3;
}

bool testResize() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec( 2 );
  vec.resize( 3 );
  if( vec.size() != 3 )
    return false;

  vec[2].resize( 2, 42 );
  return vec[2].size() == 2 && vec[2][1] == 42;
}

bool testReserve() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec;
  vec.reserve( N );
  assert( vec.capacity() == N );

  auto initial_address = vec.data();
  bool reserve_failed  = false;
  bool realloc_failed  = false;
  for( int i = 0; i < N; i++ ) {
    vec.push_back( std::vector<int64_t, Allocator<int64_t>>{ i, i } );
    if( vec.data() != initial_address ) {  // the data address must not change for N pushes
      reserve_failed = true;
      break;
    }
  }

  return !reserve_failed && ( vec.size() == N );
}

bool testPushBack() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec;
  vec.push_back( { 1, 2 } );
  vec.push_back( { 3 } );

  return vec.size() == 2 && vec[1].size() == 1 && vec[1][0] == 3;
}

bool testBeginEnd() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec = {
    { 1 }, { 2 }, { 3 }
  };
  auto it = vec.begin();
  if( it == vec.end() || ( *it )[0] != 1 )
    return false;

  int sum = 0;
  for( auto it = vec.begin(); it != vec.end(); ++it ) {
    sum += ( *it )[0];
  }
  return sum == 6;
}

bool testAt() {
  std::vector<std::vector<int64_t, Allocator<int64_t>>, Allocator<std::vector<int64_t, Allocator<int64_t>>>> vec = {
    { 1 }, { 2 }, { 3 }, { 4 }
  };
  if( vec.at( 0 )[0] != 1 && vec.at( 3 )[0] != 4 ) {
    return false;
  }

  try {
    auto check = vec.at( 5 );
    return false;  // this line should never execute
  } catch( std::out_of_range& e ) {
    return true;
  }

  // return false;
}

int main() {

  assert( testReserve() );
  assert( testSize() );
  assert( testResize() );
  assert( testPushBack() );
  assert( testBeginEnd() );
  assert( testAt() );

  return 0;
}
