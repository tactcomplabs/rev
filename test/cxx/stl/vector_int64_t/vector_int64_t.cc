//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "revalloc.h"
#include <vector>

#include "rev-macros.h"

#include <cstdint>
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

// fill in pre-sized std vector -- tests out the [] access operator
static inline void fill_vector( std::vector<int64_t, Allocator<int64_t>>& v, int start_pos, int count ) {
  int end_pos = start_pos + count;
  for( int i = start_pos; i < end_pos; i++ ) {
    v[i] = i;
  }
}

// fill a vector using push_back
static inline void fill_push_back_vector( std::vector<int64_t, Allocator<int64_t>>& v, int start_num, int count ) {
  int end_num = start_num + count;
  for( int i = start_num; i < end_num; i++ ) {
    v.push_back( i );
  }
}

void test_reserve() {
  std::vector<int64_t, Allocator<int64_t>> v;
  v.reserve( N );

  assert( v.capacity() == N );
  assert( v.size() == 0 );

  auto initial_address = v.data();
  bool reserve_failed  = false;
  for( int64_t i = 0; i < N; i++ ) {
    v.push_back( i );
    if( v.data() != initial_address ) {
      reserve_failed = true;
      break;
    }
  }
  assert( v.size() == N && !reserve_failed && ( v[N / 2] == N / 2 ) )

    // Now check if the reallocation works when the vector grows larger than capacity
    v.push_back( N );
  assert( v.data() != initial_address && v.size() == ( N + 1 ) && v.capacity() >= ( N + 1 ) );
}

void test_resize() {
  std::vector<int64_t, Allocator<int64_t>> v;
  v.resize( N );

  assert( v.size() == N );
  fill_vector( v, 0, N );
  assert( v.size() == N );  // ensure that vector::size has not increased

  fill_push_back_vector( v, N, N );
  assert( v.size() == ( 2 * N ) );  // ensure that vector::size is now 2*N

  // Resize to 4*N and check
  v.resize( 4 * N );
  assert( v.size() == 4 * N );
  fill_vector( v, 2 * N, 2 * N );

  assert( v.size() == 4 * N );
}

void test_begin_and_end() {
  std::vector<int64_t, Allocator<int64_t>> v;
  fill_push_back_vector( v, 0, N );

  int64_t i = 0;
  for( auto iter = v.begin(); iter < v.end(); iter++, i++ ) {
    assert( *iter == i );
  }
}

void test_erase() {
  std::vector<int64_t, Allocator<int64_t>> v;
  fill_push_back_vector( v, 0, N );
  assert( v.size() == N );

  // erase the first N / 2 elements one at a time
  for( int i = 0; i < ( N / 2 ); i++ ) {
    v.erase( v.begin() );
  }
  assert( v.size() == ( N / 2 ) );
  assert( v[0] == ( N / 2 ) );

  // test erase via iterator range
  v.erase( v.begin(), v.end() );
  assert( v.size() == 0 );
}

void test_at() {
  std::vector<int64_t, Allocator<int64_t>> v;
  fill_push_back_vector( v, 0, N );

  int check = -1;
  try {
    v.at( N + 1 ) = N + 1;
    check         = 0;  // this should not run
  } catch( const std::out_of_range& e ) {
    assert( check == -1 );
  }
}

int main() {

  test_reserve();
  test_resize();
  test_begin_and_end();
  test_erase();
  test_at();

  return 0;
}
