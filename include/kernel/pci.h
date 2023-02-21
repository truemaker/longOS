#pragma once
#include <typedef.h>
namespace PCI {
    #define PCI_CONFIG_ADDRESS 0xCF8
    #define PCI_CONFIG_DATA 0xCFC
    typedef struct pci_device_h {
        uint16_t deviceID;
        uint16_t vendorID;
        uint16_t status;
        uint16_t command;
        uint8_t class_code;
        uint8_t subclass;
        uint8_t prog_if;
        uint8_t revisionID;
        uint8_t BIST;
        uint8_t header_type;
        uint8_t latency_timer;
        uint8_t cache_line_size;
    } header_pci_device_t;

    typedef struct pci_device_h0 {
        header_pci_device_t base_header;
        uint32_t BAR0;
        uint32_t BAR1;
        uint32_t BAR2;
        uint32_t BAR3;
        uint32_t BAR4;
        uint32_t BAR5;
        uint32_t card_bus_cis_ptr;
        uint16_t subsystemID;
        uint16_t subsystem_vendorID;
        uint32_t expansion_rom_base;
        uint16_t reserved0;
        uint8_t reserved1;
        uint8_t capabilities_ptr;
        uint32_t reserved2;
        uint8_t max_latency;
        uint8_t min_grant;
        uint8_t interrupt_pin;
        uint8_t interrupt_line;
    } header0_pci_device_t;

    typedef struct pci_device {
        uint8_t bus;
        uint8_t device;
        uint8_t function;
    } pci_device_t;
    

    uint16_t read_config_word(uint8_t bus,uint8_t device, uint8_t func, uint8_t offset);
    void print_pci(void);
    void print_bus(uint8_t bus);
    void print_device(uint8_t bus, uint8_t device);
    void print_function(uint8_t bus, uint8_t device, uint8_t func);
    void iterate_pci(void);
    void iterate_bus(uint8_t bus);
    void iterate_device(uint8_t bus, uint8_t device);
    void iterate_function(uint8_t bus, uint8_t device, uint8_t func);
}