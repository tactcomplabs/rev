/*
 * Copyright (c) 2021 University of Chicago and Argonne National Laboratory
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author - Jose M Monsalve Diaz
 *
 * Helper functions to add debug comments, error messages and others
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO set using compiler flags only
#ifndef ASST
#define ASST
#endif

#ifdef ASST
#include "rev-macros.h"
#include "syscalls.h"

#undef assert
#define assert TRACE_ASSERT
#endif

#ifndef DEBUG_MODE
#define rev_udrt_print( ... )
#endif

#define __FILENAME__ \
  ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )

// Macro for output of information, warning and error messages
#ifdef DEBUG_MODE

#ifndef ASST
#define UPDOWN_ASSERT( assertion, message, ... )              \
  {                                                           \
    for( ; !( assertion );                                    \
         printf( "[UPDOWN_ASSERT_FAIL: %s:%i] " message "\n", \
                 __FILENAME__,                                \
                 __LINE__,                                    \
                 ##__VA_ARGS__ ),                             \
         fflush( stderr ),                                    \
         fflush( stdout ),                                    \
         assert( 0 ) ) {                                      \
    }                                                         \
  }
#else
#define UPDOWN_ASSERT( assertion, message, ... ) \
  { TRACE_ASSERT( assertion ); }
#endif

#ifndef ASST
#define UPDOWN_WARNING( message, ... )               \
  {                                                  \
    printf( "[UPDOWN_WARNING: %s:%i] " message "\n", \
            __FILENAME__,                            \
            __LINE__,                                \
            ##__VA_ARGS__ );                         \
    fflush( stderr );                                \
    fflush( stdout );                                \
  }
#else
#define UPDOWN_WARNING( message, ... )        \
  {                                           \
    rev_udrt_print( "UPDOWN_WARNING: " );     \
    rev_udrt_print( message, ##__VA_ARGS__ ); \
  }
#endif

#ifndef ASST
#define UPDOWN_WARNING_IF( condition, message, ... )   \
  {                                                    \
    if( condition ) {                                  \
      printf( "[UPDOWN_WARNING: %s:%i] " message "\n", \
              __FILENAME__,                            \
              __LINE__,                                \
              ##__VA_ARGS__ );                         \
      fflush( stderr );                                \
      fflush( stdout );                                \
    }                                                  \
  }
#else
#define UPDOWN_WARNING_IF( condition, message, ... ) \
  {                                                  \
    if( condition )                                  \
      rev_udrt_print( "UPDOWN_WARNING: " );          \
    rev_udrt_print( message, ##__VA_ARGS__ );        \
  }
#endif

#ifndef ASST
#define UPDOWN_ERROR( message, ... )                \
  {                                                 \
    fprintf( stderr,                                \
             "[UPDOWN_ERROR: %s:%i] " message "\n", \
             __FILENAME__,                          \
             __LINE__,                              \
             ##__VA_ARGS__ );                       \
    fflush( stderr );                               \
    fflush( stdout );                               \
    assert( 0 && message );                         \
  }
#define UPDOWN_ERROR_IF( condition, message, ... )    \
  {                                                   \
    if( condition ) {                                 \
      fprintf( stderr,                                \
               "[UPDOWN_ERROR: %s:%i] " message "\n", \
               __FILENAME__,                          \
               __LINE__,                              \
               ##__VA_ARGS__ );                       \
      fflush( stderr );                               \
      fflush( stdout );                               \
      assert( 0 && message );                         \
    }                                                 \
  }
#else
#define UPDOWN_ERROR( message, ... ) \
  { TRACE_ASSERT( 0 ); }
#define UPDOWN_ERROR_IF( condition, message, ... ) \
  {                                                \
    if( condition ) {                              \
      TRACE_ASSERT( 0 );                           \
    }                                              \
  }
#endif

#ifndef ASST
#define UPDOWN_INFOMSG( message, ... )            \
  {                                               \
    printf( "[UPDOWN_INFO: %s:%i] " message "\n", \
            __FILENAME__,                         \
            __LINE__,                             \
            ##__VA_ARGS__ );                      \
    fflush( stderr );                             \
    fflush( stdout );                             \
  }
#define UPDOWN_INFOMSG_IF( condition, message, ... ) \
  {                                                  \
    if( condition ) {                                \
      printf( "[UPDOWN_INFO: %s:%i] " message "\n",  \
              __FILENAME__,                          \
              __LINE__,                              \
              ##__VA_ARGS__ );                       \
      fflush( stderr );                              \
      fflush( stdout );                              \
    }                                                \
  }
#else
#define UPDOWN_INFOMSG( message, ... ) \
  { rev_udrt_print( message, ##__VA_ARGS__ ); }
#define UPDOWN_INFOMSG_IF( condition, message, ... ) \
  {                                                  \
    if( condition ) {                                \
      rev_udrt_print( message, ##__VA_ARGS__ );      \
    }                                                \
  }
#endif

#else
#define UPDOWN_ASSERT( assertion, message, ... ) \
  {}
#define UPDOWN_WARNING( message, ... ) \
  {}
#define UPDOWN_WARNING_IF( message, ... ) \
  {}
#define UPDOWN_ERROR( message, ... ) \
  {}
#define UPDOWN_ERROR_IF( message, ... ) \
  {}
#define UPDOWN_INFOMSG( message, ... ) \
  {}
#define UPDOWN_INFOMSG_IF( message, ... ) \
  {}
#endif  // END IF DEBUG_MODE
