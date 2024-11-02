#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"


#define FIRST_NIBBLE_MASK 0xF000
#define SECOND_NIBBLE_MASK 0xF00
#define THIRD_NIBBLE_MASK 0xF0
#define FOURTH_NIBBLE_MASK 0xF

int main() {

    uint8_t num = 10;
    uint8_t first_digit = num / 100;
    uint8_t second_digit = (num - (100*first_digit)) / 10;
    uint8_t third_digit = num - (100*first_digit) - (10*second_digit);
    printf("%d\t%d\t%d\n", first_digit, second_digit, third_digit);


}