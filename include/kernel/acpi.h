#pragma once
#include <typedef.h>

namespace ACPI {
    enum {
        ACPI_ADDRESS_SPACE_RAM,
        ACPI_ADDRESS_SPACE_PORT,
    };

    typedef struct acpi_address {
        uint8_t address_space;
        uint8_t bit_width;
        uint8_t bit_offset;
        uint8_t access_size;
        uint64_t address;
    } __attribute__((packed)) acpi_address_t;

    typedef struct RSDP {
        char signature[8];
        uint8_t checksum;
        char oemid[6];
        uint8_t revision;
        uint32_t rsdt;
    } __attribute__((packed)) RSDP_t;

    typedef struct RSDP20 {
        RSDP_t first;
        uint32_t length;
        uint64_t xsdt;
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } __attribute__((packed)) RSDP20_t;

    typedef struct sdt_header {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oem_id[6];
        char oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
    } __attribute__((packed)) sdt_header_t;

    typedef struct rsdt {
        sdt_header_t header;
        uint32_t *ptr2sdt;
    } __attribute__((packed)) rsdt_t;

    typedef struct xsdt {
        sdt_header_t header;
        uint32_t *ptr2sdt;
    } __attribute__((packed)) xsdt_t;

    typedef struct fadt {
        sdt_header_t header;
        uint32_t firmare_ctrl;
        uint32_t dsdt;
        uint8_t reserved; // ACPI 1.0
        uint8_t preferred_power_management_profile;
        uint16_t sci_interrupt;
        uint32_t smi_command_port;
        uint8_t acpi_enable;
        uint8_t acpi_disable;
        uint8_t s4bios_req;
        uint8_t pstate_control;
        uint32_t pm1a_event_block;
        uint32_t pm1b_event_block;
        uint32_t pm1a_control_block;
        uint32_t pm1b_control_block;
        uint32_t pm2_control_block;
        uint32_t pm_timer_block;
        uint32_t gpe0_block;
        uint32_t gpe1_block;
        uint8_t  pm1_event_length;
        uint8_t  pm1_control_length;
        uint8_t  pm2_control_length;
        uint8_t  pm_timer_length;
        uint8_t  gpe0_length;
        uint8_t  gpe1_length;
        uint8_t  gpe1_base;
        uint8_t  cstate_control;
        uint16_t worst_c2_latency;
        uint16_t worst_c3_latency;
        uint16_t flush_size;
        uint16_t flush_stride;
        uint8_t  duty_offset;
        uint8_t  duty_width;
        uint8_t  day_alarm;
        uint8_t  month_alarm;
        uint8_t  century;
    
        uint16_t boot_architecture_flags; // ACPI 2.0+
    
        uint8_t  reserved2;
        uint32_t flags;
    
        acpi_address_t reset_reg;
    
        uint8_t  reset_value;
        uint8_t  reserved3[3];
    
        // Everything below ACPI 2.0+
        uint64_t                x_firmware_control;
        uint64_t                x_dsdt;
    
        acpi_address_t x_pm1a_event_block;
        acpi_address_t x_pm1b_event_block;
        acpi_address_t x_pm1a_control_block;
        acpi_address_t x_pm1b_control_block;
        acpi_address_t x_pm2_control_block;
        acpi_address_t x_pm_timer_block;
        acpi_address_t x_gpe0_block;
        acpi_address_t x_gpe1_block;
    } __attribute__((packed)) fadt_t;

    void init_acpi(void);
    RSDP_t *get_rsdp(void);
    bool check_rsdp(RSDP_t*);
    bool check_rsdp(RSDP20_t*);
    bool check_version(RSDP_t*);
    sdt_header_t* get_table(rsdt_t*,char*);
    sdt_header_t* get_table(xsdt_t*,char*);
    sdt_header_t* get_table(sdt_header_t*,uint8_t,uint64_t,char*); // Unsafe
    sdt_header_t* get_table(char*);
    void print_sdt(sdt_header_t* header);
    void print_address(acpi_address_t);
    void shutdown(void);
    void enable_acpi(void);
    void detect_hardware(void);
}