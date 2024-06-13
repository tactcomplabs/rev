/*
 * argv.c
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

[[gnu::noipa]] void foo() {
  volatile char data[81];
  data[2] = '\0';
}

int main( int argc, char** argv ) {
  foo();
  return argv[0][0] != 'a';
}
