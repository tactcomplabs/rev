#include <iostream>
#include <updown.h>

int main() {
  // Create an empty operands_t
  UpDown::operands_t empty_ops;

  // Create an operand_t with operands
  UpDown::word_t     ops_data[] = { 1, 2, 3, 4 };
  UpDown::operands_t set_ops( 4, ops_data );

  // Create an event with a continuation
  UpDown::operands_t set_ops_cont( 4, ops_data, 99 );
  for( uint8_t i = 0; i < set_ops_cont.get_NumOperands() + 1; i++ )
    set_ops_cont.get_Data()[i];
  return 0;
}
