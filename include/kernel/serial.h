#pragma once
#include <typedef.h>
namespace serial {
    bool init_serial();
    char read_serial();
    void write_serial(char c);
    void write_serial(char* c,uint32_t len);
}