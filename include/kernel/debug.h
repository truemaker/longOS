#pragma once
#include <typedef.h>

struct stackframe {
  struct stackframe* ebp;
  uint64_t rip;
};

void trace(uint32_t frames);
void trace(uint32_t frames,struct stackframe *stk);