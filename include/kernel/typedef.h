#pragma once
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
typedef unsigned long int size_t;
typedef uint64_t uintptr_t;

#define NULL 0
#define asm __asm__
#define volatile __volatile__

#define ALIGN (sizeof(size_t))

#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)
#define UCHAR_MAX 255