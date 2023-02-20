#include <memory.h>
#include <heap.h>
#include <typedef.h>
#include <string.h>

size_t strlen(const char * s) {
	const char * a = s;
	const size_t * w;
	for (; (uint64_t)s % ALIGN; s++) {
		if (!*s) {
			return s-a;
		}
	}
	for (w = (const size_t *)s; !HASZERO(*w); w++);
	for (s = (const char *)w; *s; s++);
	return s-a;
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