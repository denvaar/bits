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
  uint32_t j_start = (i - 1u) * l + 1u;
  uint32_t j_end = i * l;

  if (j_start > j_end) return 0;

  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.
    return (bit_array[x] >> (j_start % BITS_PER_TYPE)) & ((1u << (j_end - j_start + 1u)) - 1u);
  }

  // encoded integer falls between two consecutive cells.
  return (bit_array[y] >> (j_start % BITS_PER_TYPE)) | (bit_array[x] & ((1u << ((j_end + 1u) % BITS_PER_TYPE)) - 1u)) << (BITS_PER_TYPE - (j_start % BITS_PER_TYPE));
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

  printf("\n");
}

int get_bit(uint32_t bit_array[], uint32_t idx) {
  uint32_t item_idx = floor(idx / (sizeof(uint32_t) * CHAR_BIT * 1.0));
  uint32_t item = bit_array[item_idx];
  uint32_t mask = 1u << (idx % (sizeof(item) * CHAR_BIT));

  return (item & mask) != 0;
}

void write_value(uint32_t *bit_array, uint32_t l, uint32_t i, uint32_t value) {
  uint32_t j_start = (i - 1u) * l + 1u;
  uint32_t j_end = i * l;

  if (j_start > j_end) return;

  int x = j_end / BITS_PER_TYPE;
  int y = j_start / BITS_PER_TYPE;

  if (x == y) {
    // encoded integer falls within the boundaries
    // of a cell.
    bit_array[x] &= ~(((1u << (j_end - j_start + 1u)) - 1u) << (j_start % BITS_PER_TYPE));
    bit_array[x] |= value << (j_start % BITS_PER_TYPE);
  } else {
    // encoded integer falls between two consecutive cells.
    bit_array[y] = (bit_array[y] & ((1u << (j_start % BITS_PER_TYPE)) - 1u)) | (value << (j_start % BITS_PER_TYPE));
    bit_array[x] = (bit_array[x] & ~((1u << ((j_end + 1u) % BITS_PER_TYPE)) - 1u)) | (value >> (BITS_PER_TYPE - (j_start % BITS_PER_TYPE)));
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
    b[i] = UINT32_MAX;
  }

  // compute the cumulative ranks and store in r1.
  uint32_t r1_chunk_size = ceil(log2(n) * log2(n));
  // number of bits required by r1
  const uint32_t r1_bits = ceil(log2(n) * (n / (r1_chunk_size)));
  // number of uint32_t's needed to store the bits
  const uint32_t r1_elements = ceil((r1_bits / (CHAR_BIT * 1.0)) / sizeof(uint32_t));
  // elements of r1 will be encoded using this many bits
  const uint32_t r1_element_width = ceil(log2(n));

  printf("bit array, r1, out of %u uint32_t's to store %u bits\n", r1_elements, r1_bits);

  uint32_t *r1 = malloc(sizeof(uint32_t) * r1_elements);

  for (uint32_t i = 0u; i < r1_elements; i++) r1[i] = 0u;

  // compute the relative ranks and store in r2.
  uint32_t r2_chunk_size = ceil(0.5 * log2(n));
  // number of bits required by r2
  const uint32_t r2_bits = ceil((4 * n * log2(log2(n))) / log2(n));
  // number of uint32_t's needed to store the bits
  const uint32_t r2_elements = ceil((r2_bits / (CHAR_BIT * 1.0)) / sizeof(uint32_t));
  // elements of r2 will be encoded using this many bits
  const uint32_t r2_element_width = ceil(log2(r1_chunk_size));

  printf("bit array, r2, out of %u uint32_t's to store %u bits\n", r2_elements, r2_bits);

  uint32_t *r2 = malloc(sizeof(uint32_t) * r2_elements);

  for (uint32_t i = 0u; i < r2_elements; i++) r2[i] = 0u;

  uint32_t chunk_rank = 0u;
  uint32_t subchunk_rank = 0u;

  printf("%u\n", r2_bits / r2_element_width);

  for (int i = 0; i < n; i++) {
    uint32_t current_chunk = ceil(i / r1_chunk_size);
    uint32_t current_subchunk = ceil(i / r2_chunk_size);

    if (i % r1_chunk_size == 0) {
      write_value(r1, r1_element_width, current_chunk, chunk_rank);
      subchunk_rank = 0u;
    }

    if (i % r2_chunk_size == 0) {
      write_value(r2, r2_element_width, current_subchunk, subchunk_rank);
      printf("%u,", current_subchunk);
    }

    chunk_rank += get_bit(b, i);
    subchunk_rank += get_bit(b, i);
  }

  printf("b=");
  print_bit_array(b, n_elements, BITS_PER_TYPE);
  printf("r1=");
  print_bit_array(r1, r1_elements, r1_element_width);
  printf("cumulative ranks=");
  print_values(r1, r1_element_width, ceil(n / r1_chunk_size));
  printf("r2=");
  print_bit_array(r2, r2_elements, r2_element_width);
  printf("relative ranks=");
  print_values(r2, r2_element_width, ceil(n / r2_chunk_size));

  free(r2);
  free(r1);
  free(b);

  return 0;
}
