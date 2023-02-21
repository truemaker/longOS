#include <memory.h>
#include <heap.h>
#include <typedef.h>
#include <string.h>
#include <vga.h>

size_t strlen(const char * str) {
	size_t len = 0;
    while (*str)
    {
        ++len;
        ++str;
    }

    return len;
}

bool strcmp(char* a, char* b) {
	if (strlen(a) != strlen(b)) return false;
	uint64_t len = strlen(a);
	for (uint64_t i = 0; i < len; i++) {
		if (a[i]!=b[i]) return false;
	}
	return true;
}

char * strdup(const char * c) {
	char * out = (char*)heap::malloc(strlen(c) + 1);
	memcpy(out, (void*)c, strlen(c)+1);
	return out;
}

uint64_t oct2bin(uint8_t *str, int size) {
	uint64_t n = 0;
    uint8_t *c = str;
    while (size-- > 0) {
	    n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

uint16_t udigits(uint64_t i, uint64_t base) {
	uint64_t v = i;
	uint16_t d = 0;
	while (v) {
		v /= base;
		d++;
	}
	return d;
} 

char* uitos(uint64_t i, uint64_t base) {
	if (base > 10) { print("[STRING] base > 10 is not supported yet\n\r"); return 0; }
	if (base == 1) { print("[STRING] base == 1 is illegal\n\r"); return 0; }
	uint64_t d = udigits(i,base);
	char* s = (char*)heap::malloc(d+1);
	uint64_t v = i;
	for (uint16_t j = 0; j < d; j++) {
		s[d-j-1] = (v % base)+'0';
		v /= base;
	}
	s[d] = 0;
	return s;
}

char* strcat(char* a, char* b) {
	char* s = (char*)heap::malloc(strlen(a)+strlen(b)+1);
	strcpy(s,a);
	strcpy(s+strlen(a),b);
	s[strlen(a)+strlen(b)] = 0;
	return s;
}

void strcpy(char* a, char* b) {
	uint64_t l = strlen(a) > strlen(b) ? strlen(b) : strlen(a);
	memcpy(a,b,l);
}