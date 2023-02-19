#pragma once
#include <typedef.h>
namespace PCI {
    #define PCI_CONFIG_ADDRESS 0xCF8
    #define PCI_CONFIG_DATA 0xCFC
    uint16_t read_config_word(uint8_t bus,uint8_t device, uint8_t func, uint8_t offset);
    void print_pci(void);
    void print_bus(uint8_t bus);
    void print_device(uint8_t bus, uint8_t device);
    void print_function(uint8_t bus, uint8_t device, uint8_t func);
}