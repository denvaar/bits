/*
 * Toy program that demonstrates how to treat a single
 * `uint64_t` value as an array of bits.
 *
 * Values can be "packed" into the bit array by
 * encoding each one using a fixed length of bits.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define BIT_OFFSET 6
#define BIT_WIDTH_MASK 63ULL

void print_bit_array(uint64_t *bit_array) {
  for (int i = 0; i < sizeof(*bit_array) * CHAR_BIT; i++) {
    printf("%llu", (*bit_array >> i) & 1ULL);

    if ((i + 1) % BIT_OFFSET == 0) {
      printf(" ");
    }
  }

  printf("\n\n");
}

void print_values(uint64_t *bit_array, int number_of_items) {
  uint64_t offset = 0;
  uint64_t value;

  printf("[");

  for (int i = 0; i < number_of_items; i++) {
    value = (*bit_array >> offset) & BIT_WIDTH_MASK;
    offset += BIT_OFFSET;

    printf("%llu", value);

    if (i < number_of_items - 1) {
      printf(", ");
    }
  }

  printf("] (stored within %llu)\n", *bit_array);
}

void set_value(uint64_t *bit_array, uint64_t value, unsigned int idx) {
  uint64_t shift = idx * BIT_OFFSET;
  uint64_t mask = BIT_WIDTH_MASK << shift;

  // clear it first
  *bit_array = (*bit_array & ~mask) | (0ULL & mask);
  *bit_array |= (value << shift);
}

int main(void) {
  uint64_t bit_array = 0;

  print_bit_array(&bit_array);

  const int number_of_items = 10;
  uint64_t values_to_insert[number_of_items] = {63, 2, 0, 1, 3, 10, 7, 33, 1, 27};
  unsigned int offset = 0;
  uint64_t n;

  // append the values, each encoded using `BIT_OFFSET` number of bits.
  for (int i = 0; i < number_of_items; i++) {
    n = values_to_insert[i];
    bit_array |= (n << offset);
    offset += BIT_OFFSET;

    printf("insert %llu ...\n", n);
    print_bit_array(&bit_array);
  }

  print_values(&bit_array, 10);

  set_value(&bit_array, 14, 0);
  print_values(&bit_array, 10);

  set_value(&bit_array, 55, 5);
  print_values(&bit_array, 10);

  set_value(&bit_array, 22, 9);
  print_values(&bit_array, 10);

  return 0;
}
