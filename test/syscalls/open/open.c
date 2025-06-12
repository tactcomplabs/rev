/*
 * open.c
 *
 * Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

/*
  https://www.man7.org/linux/man-pages/man2/open.2.html

  #include <fcntl.h>

  int open(const char *pathname, int flags, ...
            // mode_t mode // );

  int creat(const char *pathname, mode_t mode);

  int openat(int dirfd, const char *pathname, int flags, ...
            // mode_t mode // );

  // Documented separately, in openat2(2)
  int openat2(int dirfd, const char *pathname,
            const struct open_how *how, size_t size);

  *mode: r[b][+], w[b][+], a[b][+]

*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef HOST_TARGET
// REV fast printf and tracing macros
#include "rev-macros.h"
#include "syscalls.h"
#define printf rev_fast_printf
#undef assert
#define assert TRACE_ASSERT
#else
#define TRACE_ON
#define TRACE_OFF
#endif

#define xstr( s )         str( s )
#define str( s )          #s

#define TMP1              "hello.tmp"
#define NON_EXISTENT_FILE "non_existent_file.fubar"
#define DATA_CSV          "data.csv"
#define DATA_BIN          "data.bin"

int main() {

  int a     = 0;
  int b     = 0;
  int c     = 0;
  int count = 0;

  FILE* fp  = NULL;
  int   FD  = -1;
  int   rc  = -1;

  // open() related flags to be translated to/from RV OS to Host OS
  printf( "fcntl oflags - required/tested \n" );
  printf( "O_RDONLY=0x%" PRIx32 "\n", O_RDONLY );
  printf( "O_WRONLY=0x%" PRIx32 "\n", O_WRONLY );
  printf( "O_RDWR=0x%" PRIx32 "\n", O_RDWR );
  // printf( "O_ACCMODE=0x%" PRIx32 "\n", O_ACCMODE );
  assert( O_ACCMODE == ( O_WRONLY | O_RDWR ) );

  printf( "fcntl oflags - optional/tested \n" );
  printf( "O_APPEND=0x%" PRIx32 "\n", O_APPEND );
  printf( "O_CREAT=0x%" PRIx32 "\n", O_CREAT );
  printf( "O_TRUNC=0x%" PRIx32 "\n", O_TRUNC );

  printf( "fcntl oflags - optional/untested \n" );
  printf( "O_CLOEXEC=0x%" PRIx32 "\n", O_CLOEXEC );  // close-on-exec flag
#ifndef HOST_TARGET
  printf( "O_DIRECT=0x%" PRIx32 "\n", O_DIRECT );  // minimize cache effects
#endif
  printf( "O_DIRECTORY=0x%" PRIx32 "\n", O_DIRECTORY );  // fail if not directory
  printf( "O_EXCL=0x%" PRIx32 "\n", O_EXCL );            // Ensure this call creates the file
  printf( "O_NOCTTY=0x%" PRIx32 "\n", O_NOCTTY );        // if pathname is term dev do not become controlling term
  printf( "O_NOFOLLOW=0x%" PRIx32 "\n", O_NOFOLLOW );    // open fails if basename is symlink
  printf( "O_NONBLOCK=0x%" PRIx32 "\n", O_NONBLOCK );    // open in non-blocking mode when possible
  printf( "O_SYNC=0x%" PRIx32 "\n", O_SYNC );            // write ops complete according to sync IO -data- integrity

  // printf( "fcntl oflags - documented by not available\n" ); // check RV, MacOS. TODO ubuntu
  // printf( "O_ASYNC=0x%" PRIx32 "\n", O_ASYNC );     // signal-driven IO
  // printf( "O_DSYNC=0x%" PRIx32 "\n", O_DSYNC );     // writ ops complete according to sync io -file- integrity
  // printf( "O_LARGEFILE=0x%" PRIx32 "\n", O_LARGEFILE ); // allow off64_t files to be opened
  // printf( "O_NOATIME=0x%" PRIx32 "\n", O_NOATIME ); // Do not update st_atime
  // printf( "O_NDELAY=0x%" PRIx32 "\n", O_NDELAY );     // same as O_NONBLOCK
  // printf( "O_PATH=0x%" PRIx32 "\n", O_PATH );         // obatin file desc for location and file desc operations
  // printf( "O_TMPFILE=0x%" PRIx32 "\n", O_TMPFILE );   // Create an  unnamed temp regular file

  // lseek() related flags
  printf( "lseek flags. Tested\n" );
  printf( "SEEK_SET=0x%" PRIx32 "\n", SEEK_SET );  // The file offset is set to offset bytes
  printf( "lseek flags. untested\n" );
  printf( "SEEK_CUR=0x%" PRIx32 "\n", SEEK_CUR );  // The file offset is set to its current location plus offset bytes.
  printf( "SEEK_END=0x%" PRIx32 "\n", SEEK_END );  // The file offset is set to the size of the file plus offset bytes.

  printf( "open file creation [O_WRONLY | O_CREAT | O_TRUNC]\n" );
  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  printf( "open(" xstr( TMP1 ) ", 0x%" PRIx32 ", 0664)\n", flags );
  FD = open( TMP1, flags, 0664 );
  assert( FD != -1 );
  rc = close( FD );
  assert( rc == 0 );
  printf( "...passed\n" );

  printf( "open existing file and write to it [O_WRONLY]\n" );
  flags = O_WRONLY;
  printf( "open(" xstr( TMP1 ) ", 0x%" PRIx32 ")\n", flags );
  FD = open( TMP1, flags, 0664 );
  assert( FD != -1 );
  const char* hello      = "hello\n";
  ssize_t     hellobytes = write( FD, hello, strlen( hello ) );
  printf( "wrote %d bytes\n", hellobytes );
  assert( hellobytes == 6 );
  rc = close( FD );
  assert( rc == 0 );
  printf( "...passed\n" );

  printf( "open file for reading [O_RDONLY]\n" );
  flags = O_RDONLY;
  printf( "open(" xstr( TMP1 ) ", 0x%" PRIx32 ")\n", flags );
  FD = open( TMP1, flags );
  assert( FD != -1 );
  char hellobuf[10] = { 0 };
  hellobytes        = read( FD, hellobuf, sizeof( hellobuf ) );
#ifdef HOST_TARGET
  printf( "Read %d bytes: %s\n", (unsigned) hellobytes, hellobuf );
#else
  printf( "Read %d bytes\n", (unsigned) hellobytes );
#endif
  assert( hellobytes == 6 );
  hellobytes = read( FD, hellobuf, sizeof( hellobuf ) );
  assert( hellobytes == 0 );
  rc = close( FD );
  assert( rc == 0 );
  printf( "...passed\n" );

  printf( "open existing file for read/write (append) [O_RDWR | O_APPEND]\n" );
  flags = O_RDWR | O_APPEND;
  printf( "open(" xstr( TMP1 ) ", 0x%" PRIx32 ")\n", flags );
  FD = open( TMP1, flags );
  assert( FD != -1 );
  // the write
  const char* goodbye      = "goodbye\n";
  ssize_t     goodbyebytes = write( FD, goodbye, strlen( goodbye ) );
  printf( "wrote %d bytes\n", goodbyebytes );
  assert( goodbyebytes == 8 );
  // rewind the pointer and read
  lseek( FD, 0, SEEK_SET );
  char goodbyebuf[15] = { 0 };
  goodbyebytes        = read( FD, goodbyebuf, sizeof( goodbyebuf ) );
#ifdef HOST_TARGET
  printf( "Read %d bytes: %s\n", (unsigned) goodbyebytes, goodbyebuf );
#else
  printf( "Read %d bytes\n", (unsigned) goodbyebytes );
#endif
  assert( goodbyebytes == 14 );
  rc = close( FD );
  assert( rc == 0 );
  printf( "...passed\n" );

  printf( "open existing file for read/write (trunc) [O_RDWR | O_TRUNC]\n" );
  flags = O_RDWR | O_TRUNC;
  printf( "open(" xstr( TMP1 ) ", 0x%" PRIx32 ", 0664)\n", flags );
  FD = open( TMP1, flags );
  assert( FD != -1 );
  // the write
  const char* tag      = "gutentag\n";
  ssize_t     tagbytes = write( FD, tag, strlen( tag ) );
  printf( "wrote %d bytes\n", tagbytes );
  assert( tagbytes == 9 );
  // rewind the pointer from the end of the file 4 bytes and read
  lseek( FD, -4, SEEK_END );
  char tagbuf[10] = { 0 };
  tagbytes        = read( FD, tagbuf, sizeof( tagbuf ) );
#ifdef HOST_TARGET
  printf( "Read %d bytes: %s\n", (unsigned) tagbytes, tagbuf );
#else
  printf( "Read %d bytes\n", (unsigned) tagbytes );
#endif
  assert( tagbytes == 4 );
  // rewind the pointer from the current location 9 bytes and read
  lseek( FD, -9, SEEK_CUR );
  tagbytes = read( FD, tagbuf, sizeof( tagbuf ) );
#ifdef HOST_TARGET
  printf( "Read %d bytes: %s\n", (unsigned) tagbytes, tagbuf );
#else
  printf( "Read %d bytes\n", (unsigned) tagbytes );
#endif
  assert( tagbytes == 9 );

  rc = close( FD );
  assert( rc == 0 );
  printf( "...passed\n" );

  return 0;
}
