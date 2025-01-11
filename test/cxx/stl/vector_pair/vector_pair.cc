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

#define N 10
#define M 20

bool testSize() {
  std::vector<std::pair<int64_t, int64_t>, Allocator<std::pair<int64_t, int64_t>>> vec = {
    {1, 2},
    {3, 4}
  };
  if( vec.size() != 2 )
    return false;

  vec.push_back( { 5, 6 } );
  return vec.size() == 3;
}

bool testReserve() {
  std::vector<std::pair<int64_t, int64_t>, Allocator<std::pair<int64_t, int64_t>>> vec;
  vec.reserve( N );
  if( vec.capacity() != N ) {
    return false;
  }

  auto initial_address = vec.data();
  bool reserve_failed  = false;
  for( int i = 0; i < N; i++ ) {
    vec.push_back( std::pair<int64_t, int64_t>( i, i ) );

    // the vector backing store address should not change
    if( vec.data() != initial_address ) {
      reserve_failed = true;
      break;
    }
  }

  return !reserve_failed && ( vec.size() == N );
}

bool testResize() {
  std::vector<std::pair<int64_t, int64_t>, Allocator<std::pair<int64_t, int64_t>>> vec( 2 );
  vec.resize( 4 );
  if( vec.size() != 4 )
    return false;

  // Newly added pairs should be initialized to (0, 0)
  return vec[2].first == 0 && vec[2].second == 0 && vec[3].first == 0 && vec[3].second == 0;
}

bool testPushBack() {
  std::vector<std::pair<int64_t, int64_t>, Allocator<std::pair<int64_t, int64_t>>> vec;
  vec.push_back( { 1, 2 } );
  vec.push_back( { 3, 4 } );

  return vec.size() == 2 && vec[1].first == 3 && vec[1].second == 4;
}

bool testBeginEnd() {
  std::vector<std::pair<int64_t, int64_t>, Allocator<std::pair<int64_t, int64_t>>> vec = {
    {1, 1},
    {2, 2},
    {3, 3}
  };
  auto it = vec.begin();
  if( it == vec.end() || it->first != 1 || it->second != 1 )
    return false;

  int64_t sumFirst = 0, sumSecond = 0;
  for( auto it = vec.begin(); it != vec.end(); ++it ) {
    sumFirst += it->first;
    sumSecond += it->second;
  }
  return sumFirst == 6 && sumSecond == 6;
}

int main() {

  assert( testSize() );
  assert( testPushBack() );
  assert( testReserve() );
  assert( testResize() );
  assert( testBeginEnd() );

  return 0;
}
