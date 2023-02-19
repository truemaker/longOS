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

char * strdup(const char * c) {
	char * out = (char*)heap::malloc(strlen(c) + 1);
	memcpy(out, (void*)c, strlen(c)+1);
	return out;
}