#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BITS_PER_TYPE 32u


uint32_t read_value(uint32_t *bit_array, uint32_t l, uint32_t i) {
  // if i is the index into the "virtual" array of values,
  // then j_start is the corresponding index into the actual
  // array of bits where the integer stored at i begins to be
  // encoded.
  uint32_t j_start = ((i + 1) - 1) * l + 1;
  uint32_t j_end = (i + 1) * l;

  if (j_start > j_end) return 0;

  uint32_t r = ((j_end - 1u) % BITS_PER_TYPE) + 1u;
  uint32_t pow2 = pow(2, BITS_PER_TYPE - r);

  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.

    return bit_array[x] / pow2 % (uint32_t)pow(2, j_end - j_start + 1);
  }

  // encoded integer falls between two consecutive cells.

  uint32_t rp = (j_start - 1u) % BITS_PER_TYPE;

  return bit_array[x] / pow2 + (uint32_t)(bit_array[y] % (uint32_t)pow(2, BITS_PER_TYPE - rp)) * (uint32_t)pow(2, r);
}

void print_values(uint32_t *bit_array, uint32_t l, uint32_t n) {
  printf("[");
  for (int i = 0; i < n; i++) {
    printf("%u", read_value(bit_array, l, i));
    if (i < n - 1) printf(", ");
  }
  printf("]\n");
}

void print_bit_array(uint32_t *bit_array, int n_elements, int virtual_element_width) {
  for (int i = 0; i < sizeof(*bit_array) * CHAR_BIT * n_elements; i++) {
    printf("%u", (*bit_array >> i) & 1u);

    if ((i + 1) % virtual_element_width == 0) {
      printf(" ");
    }
  }

  printf("\n\n");
}

int get_bit(uint32_t bit_array[], unsigned int idx) {
  unsigned int item_idx = floor(idx / (sizeof(uint32_t) * CHAR_BIT * 1.0));
  uint32_t item = bit_array[item_idx];
  uint32_t mask = 1u << (idx % (sizeof(item) * CHAR_BIT));

  return (item & mask) != 0;
}

void write_value(uint32_t *bit_array, uint32_t l, uint32_t i, uint32_t value) {
  uint32_t j_start = ((i + 1) - 1) * l + 1;
  uint32_t j_end = (i + 1) * l;

  if (j_start > j_end) return;

  uint32_t r = ((j_end - 1u) % BITS_PER_TYPE) + 1u;

  uint32_t pow2 = pow(2, BITS_PER_TYPE - r);
  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.
    bit_array[x] = bit_array[x] - ((uint32_t)floor(bit_array[x] /  pow2) % (uint32_t)pow(2, j_end - j_start + 1)) * pow2;
    bit_array[x] = bit_array[x] + value * pow2;
  } else {
    // encoded integer falls between two consecutive cells.

    uint32_t rp = (j_start - 1u) % BITS_PER_TYPE;

    bit_array[x] = bit_array[x] % pow2 + (value % (uint32_t)pow(2, r)) * pow2;
    bit_array[y] = bit_array[y] - (bit_array[y] % (uint32_t)pow(2, BITS_PER_TYPE - rp)) + (uint32_t)floor(value / pow(2, r));
  }
}

int main(void) {
  // desired number of bits
  const uint32_t n = 1024u;

  // number of uint32_t's in b
  const uint32_t n_elements = ceil((n / (CHAR_BIT * 1.0)) / sizeof(uint32_t));

  printf("bit array, b, out of %u uint32_t's to store %u bits\n", n_elements, n);

  uint32_t *b = malloc(sizeof(uint32_t) * n_elements);

  for (uint32_t i = 0u; i < n_elements; i++) {
    b[i] = 4294967295u;
  }

  // compute the cumulative ranks and store in r1.
  uint32_t r1_chunk_size = log2(n) * log2(n);
  // number of bits required by r1
  const uint32_t r1_bits = ceil(log2(n) * (n / (r1_chunk_size)));
  // number of uint32_t's needed to store the bits
  const uint32_t r1_elements = ceil((r1_bits / (CHAR_BIT * 1.0)) / sizeof(uint32_t));
  // elements of r1 will be encoded using this many bits
  const uint32_t r1_element_width = ceil(log2(n));

  printf("bit array, r1, out of %u uint32_t's to store %u bits\n", r1_elements, r1_bits);

  uint32_t *r1 = malloc(sizeof(uint32_t) * r1_elements);

  for (uint32_t i = 0u; i < r1_elements; i++) r1[i] = 0u;

  uint32_t chunk_rank = 0u;

  for (int i = 0; i < n; i++) {
    uint32_t current_chunk = ceil(i / r1_chunk_size);

    if (i % r1_chunk_size == 0) {
      write_value(r1, r1_element_width, current_chunk, chunk_rank);
    }

    chunk_rank += get_bit(b, i);
  }

  printf("b=");
  print_bit_array(b, n_elements, BITS_PER_TYPE);
  printf("r1=");
  print_bit_array(r1, r1_elements, r1_element_width);
  printf("cumulative ranks=");
  print_values(r1, r1_element_width, ceil(n / r1_chunk_size));

  free(r1);
  free(b);

  return 0;
}
