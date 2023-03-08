#pragma once
#include <typedef.h>

size_t strlen(const char * );
bool strcmp(char * ,char * );
char * strdup(const char * c);
uint64_t oct2bin(uint8_t *str, int size);
char* uitos(uint64_t,uint64_t,uint16_t min_digits = 0);
char* strcat(char*,char*);
void strcpy(char*,char*);