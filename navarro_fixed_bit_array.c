#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

// sizeof(unsigned int) * CHAR_BIT
#define BITS_PER_TYPE 32u

int read_bit(unsigned int *bit_array, unsigned int j) {
  return (bit_array[j / BITS_PER_TYPE] >> (j & (BITS_PER_TYPE - 1))) & 1u;
}

void set_bit(unsigned int *bit_array, unsigned int j) {
  bit_array[j / BITS_PER_TYPE] |= 1u << (j & (BITS_PER_TYPE - 1));
}

void clear_bit(unsigned int *bit_array, unsigned int j) {
  bit_array[j / BITS_PER_TYPE] &= ~(1u << (j & (BITS_PER_TYPE - 1)));
}

unsigned int read_value(unsigned int *bit_array, unsigned int l, unsigned int i) {
  // if i is the index into the "virtual" array of values,
  // then j_start is the corresponding index into the actual
  // array of bits where the integer stored at i begins to be
  // encoded.
  unsigned int j_start = ((i + 1) - 1) * l + 1;
  unsigned int j_end = (i + 1) * l;

  if (j_start > j_end) return 0;

  unsigned int r = ((j_end - 1u) % BITS_PER_TYPE) + 1u;
  unsigned int pow2 = pow(2, BITS_PER_TYPE - r);

  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.

    return bit_array[x] / pow2 % (unsigned int)pow(2, j_end - j_start + 1);
  }

  // encoded integer falls between two consecutive cells.

  unsigned int rp = (j_start - 1u) % BITS_PER_TYPE;

  return bit_array[x] / pow2 + (unsigned int)(bit_array[y] % (unsigned int)pow(2, BITS_PER_TYPE - rp)) * (unsigned int)pow(2, r);
}

void write_value(unsigned int *bit_array, unsigned int l, unsigned int i, unsigned int value) {
  unsigned int j_start = ((i + 1) - 1) * l + 1;
  unsigned int j_end = (i + 1) * l;

  if (j_start > j_end) return;

  unsigned int r = ((j_end - 1u) % BITS_PER_TYPE) + 1u;

  unsigned int pow2 = pow(2, BITS_PER_TYPE - r);
  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.
    bit_array[x] = bit_array[x] - ((unsigned int)floor(bit_array[x] /  pow2) % (unsigned int)pow(2, j_end - j_start + 1)) * pow2;
    bit_array[x] = bit_array[x] + value * pow2;
  } else {
    // encoded integer falls between two consecutive cells.

    unsigned int rp = (j_start - 1u) % BITS_PER_TYPE;

    bit_array[x] = bit_array[x] % pow2 + (value % (unsigned int)pow(2, r)) * pow2;
    bit_array[y] = bit_array[y] - (bit_array[y] % (unsigned int)pow(2, BITS_PER_TYPE - rp)) + (unsigned int)floor(value / pow(2, r));
  }
}

int main(void) {
  // bit_array has n virtual elements, each encoded using l bits.
  unsigned int n = 10, l = 5;
  unsigned int capacity = ceil((l * n * 1.0) / BITS_PER_TYPE);

  // bit_array is W in fig 3.1
  unsigned int *bit_array = malloc(capacity * sizeof(unsigned int));
  bit_array[0] = 2762827861u;
  bit_array[1] = 1991065600u;

  // print unsigned ints that make up the bit array
  printf("actual array [");
  for (int i = 0; i < capacity; i++) {
    printf("%u", bit_array[i]);
    if (i < capacity - 1) printf(", ");
  }
  printf("]\n");

  // print the bits of the bit array
  for (int i = 0; i < capacity * BITS_PER_TYPE; i++) {
    printf("%d", read_bit(bit_array, i));
    if ((i + 1) % BITS_PER_TYPE == 0) printf(" ");
  }
  printf("\n");

  // print the encoded values in the bit array
  printf("encoded values [");
  for (int i = 0; i < n; i++) {
    printf("%u", read_value(bit_array, l, i));
    if (i < n - 1) printf(", ");
  }
  printf("]\n");

  // print the bits of the bit array, with respect to
  // the encoded values.
  for (int i = 0; i < capacity * BITS_PER_TYPE; i++) {
    int x = i % BITS_PER_TYPE;
    int p = (BITS_PER_TYPE - x - 1) + (i / 32) * 32;
    printf("%d", read_bit(bit_array, p));
    if ((i + 1) % l == 0) printf(" ");
  }
  printf("\n");

  printf("write 2 to index 0\n");
  write_value(bit_array, l, 0, 2u);

  printf("write 10 to index 1\n");
  write_value(bit_array, l, 1, 10u);

  printf("write 23 to index 6\n");
  write_value(bit_array, l, 6, 23u);

  printf("write 25 to index 9\n");
  write_value(bit_array, l, 9, 25u);

  // print the encoded values in the bit array
  printf("encoded values [");
  for (int i = 0; i < n; i++) {
    printf("%u", read_value(bit_array, l, i));
    if (i < n - 1) printf(", ");
  }
  printf("]\n");

  free(bit_array);

  return 0;
}
