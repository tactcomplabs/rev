// Generic simple c program

int main() {
  int a = 10;
  int b = 20;
  int c = a + b;

  for( int i = 0; i < 10; i++ ) {
    c += i;
  }
  if( c != 40 ) {
    return 1;
  } else {
    return 0;
  }
}
