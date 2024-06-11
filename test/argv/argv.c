[[gnu::noipa]] void foo() {
  volatile char data[81];
  data[2] = '\0';
}

int main( int argc, char** argv ) {
  foo();
  return argv[0][0];
}
