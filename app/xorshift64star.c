#include "xorshift64star.h"

// The state of the "randomization engine".
// Defined here so that it can be shared between files.
uint64_t xor_state[1] = {1};