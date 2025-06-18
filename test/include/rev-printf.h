//
// _rev_printf_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

// Notes
// - Reference: https://sourceware.org/newlib/libc.html#sprintf
// - See $REVHOME/test/syscall/printf for example usage
// - If the version of printf in stdio is called then rev
//   should print an error message for an unimplemented ecall.
// - Current support limited to the following formatting characters.
//   -      : flag to pad on the right
//   0      : flag to pad with 0's instead of spaces
//   [0-9*] : width fields
//   .      : precision field
//   #      : convert to alternative form
//   %      : escaped character
//   l      : long
//   c      : character
//   s      : string
//   d      : signed decimal
//   u      : unsigned decimal
//   o      : octal
//   p      : pointer
//   x      : hexadecimal

#ifndef __REV_PRINTF_H__

// #define REV_DEBUG
#ifdef REV_DEBUG
#define debug_printf rev_fast_printf
#else
#define debug_printf
#endif

//clang-format off
#include "rev-macros.h"
#include "syscalls.h"
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// ensure we always use the rev version for these
#define printf( format, ... )       rev_printf( format, ##__VA_ARGS__ )
#define sprintf( str, format, ... ) rev_sprintf( str, format, ##__VA_ARGS__ )
//clang-format on

int rev_printf( const char* fmt, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
int rev_sprintf( char* str, const char* fmt, ... ) __attribute__( ( format( printf, 2, 3 ) ) );

const char nullchar = '\0';

void printstr( const char* s ) {
  ssize_t bytes_written2 = rev_write( STDOUT_FILENO, s, strlen( s ) );
}

int rev_putchar( int ch, void** putdat ) {
  static __thread char buf[64] __attribute__( ( aligned( 64 ) ) );
  static __thread int  buflen   = 0;
  int                  putcount = buflen;
  buf[buflen]                   = ch;
  buflen++;
  // TODO revisit the algorithms for sprintf and printf. Needs more testing
  int wr_printf  = ( putdat == 0 ) && ( ch == '\n' );
  int wr_sprintf = ( putdat != 0 ) && ( ch == '\0' );
  if( wr_printf || wr_sprintf || buflen == sizeof( buf ) ) {
    if( putdat == 0 ) {
      debug_printf( "stdout<-..." );
      rev_write( STDOUT_FILENO, buf, buflen );
    } else {
      void* p = putdat;
      debug_printf( "memcpy, 0x%x, %d\n", p, buf );
      memcpy( p, buf, buflen );
    }
    buflen = 0;
    debug_printf( "rev_putchar wrote %d bytes\n", putcount );
  }
  return putcount;
}

void printhex( uint64_t x ) {
  char str[17];
  int  i;
  for( i = 0; i < 16; i++ ) {
    str[15 - i] = ( x & 0xF ) + ( ( x & 0xF ) < 10 ? '0' : 'a' - 10 );
    x >>= 4;
  }
  str[16] = 0;

  printstr( str );
}

static inline int
  printnum( void ( *putch )( int, void** ), void** putdat, unsigned long long num, unsigned base, int width, int padc ) {
  unsigned digs[sizeof( num ) * CHAR_BIT];
  int      pos   = 0;
  int      bytes = 0;

  while( 1 ) {
    digs[pos++] = num % base;
    if( num < base )
      break;
    num /= base;
  }

  while( width-- > pos )
    putch( padc, putdat );

  while( pos-- > 0 ) {
    putch( digs[pos] + ( digs[pos] >= 10 ? 'a' - 10 : '0' ), putdat );
    bytes++;
  }
  return bytes;
}

static unsigned long long getuint( va_list* ap, int lflag ) {
  if( lflag >= 2 )
    return va_arg( *ap, unsigned long long );
  else if( lflag )
    return va_arg( *ap, unsigned long );
  else
    return va_arg( *ap, unsigned int );
}

static long long getint( va_list* ap, int lflag ) {
  if( lflag >= 2 )
    return va_arg( *ap, long long );
  else if( lflag )
    return va_arg( *ap, long );
  else
    return va_arg( *ap, int );
}

static int rev_vprintfmt( void ( *putch )( int, void** ), void** putdat, const char* fmt, va_list ap ) {
  register const char* p;
  const char*          last_fmt;
  register int         ch, err;
  unsigned long long   num;
  int                  base, lflag, width, precision, altflag;
  char                 padc;

  int bytes = 0;
  debug_printf( "Entered rev_vprintfmt. putdat is %x\n", putdat );
  bytes = 0;
  while( 1 ) {
    while( ( ch = *(unsigned char*) fmt ) != '%' ) {
      if( ch == '\0' ) {
        debug_printf( "End of string. bytes=%d\n", bytes );
        if( putdat ) {
          // sprintf writes null char
          putch( ch, putdat );
        }
        return bytes;
      }
      fmt++;
      putch( ch, putdat );
      bytes++;
    }
    fmt++;

    // Process a %-escape sequence
    last_fmt  = fmt;
    padc      = ' ';
    width     = -1;
    precision = -1;
    lflag     = 0;
    altflag   = 0;
  reswitch:
    switch( ch = *(unsigned char*) fmt++ ) {

    // flag to pad on the right
    case '-': padc = '-'; goto reswitch;

    // flag to pad with 0's instead of spaces
    case '0': padc = '0'; goto reswitch;

    // width field
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      for( precision = 0;; ++fmt ) {
        precision = precision * 10 + ch - '0';
        ch        = *fmt;
        if( ch < '0' || ch > '9' )
          break;
      }
      goto process_precision;

    case '*': precision = va_arg( ap, int ); goto process_precision;

    case '.':
      if( width < 0 )
        width = 0;
      goto reswitch;

    case '#':
      altflag = 1;
      goto reswitch;

    process_precision:
      if( width < 0 )
        width = precision, precision = -1;
      goto reswitch;

    // long flag (doubled for long long)
    case 'l': lflag++; goto reswitch;

    // character
    case 'c': {
      putch( va_arg( ap, int ), putdat );
      bytes++;
      break;
    }

    // string
    case 's':
      if( ( p = va_arg( ap, char* ) ) == NULL )
        p = "(null)";
      if( width > 0 && padc != '-' )
        for( width -= strnlen( p, precision ); width > 0; width-- ) {
          putch( padc, putdat );
          bytes++;
        }
      for( ; ( ch = *p ) != '\0' && ( precision < 0 || --precision >= 0 ); width-- ) {
        putch( ch, putdat );
        bytes++;
        p++;
      }
      for( ; width > 0; width-- ) {
        putch( ' ', putdat );
        bytes++;
      }
      break;

    // (signed) decimal
    case 'd':
      num = getint( &ap, lflag );
      if( (long long) num < 0 ) {
        putch( '-', putdat );
        bytes++;
        num = -(long long) num;
      }
      base = 10;
      goto signed_number;

    // unsigned decimal
    case 'u': base = 10; goto unsigned_number;

    // (unsigned) octal
    case 'o':
      // should do something with padding so it's always 3 octits
      base = 8;
      goto unsigned_number;

    // pointer
    case 'p':
      //static_assert(sizeof(long) == sizeof(void*));
      lflag = 1;
      putch( '0', putdat );
      bytes++;
      putch( 'x', putdat );
      bytes++;
      /* fall through to 'x' */

    // (unsigned) hexadecimal
    case 'x':
      base = 16;
    unsigned_number:
      num = getuint( &ap, lflag );
    signed_number:
      bytes += printnum( putch, putdat, num, base, width, padc );
      break;

    // escaped '%' character
    case '%': {
      putch( ch, putdat );
      bytes++;
      break;
    }

    // unrecognized escape sequence - just print it literally
    default:
      putch( '%', putdat );
      bytes++;
      fmt = last_fmt;
      break;
    }
  }
  return -1;
}

int rev_printf( const char* fmt, ... ) {
  va_list ap;
  va_start( ap, fmt );
  int bytes = rev_vprintfmt( (void*) rev_putchar, 0, fmt, ap );
  va_end( ap );
  return bytes;
}

int rev_sprintf( char* str, const char* fmt, ... ) {
  va_list ap;
  va_start( ap, fmt );
  int bytes = rev_vprintfmt( (void*) rev_putchar, (void**) str, fmt, ap );
  va_end( ap );
  return bytes;
}

#endif  // __REV_PRINTF_H__
