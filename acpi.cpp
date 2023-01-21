#include <acpi.h>
#include <vga.h>
#include <memory.h>

namespace ACPI {
    bool extended = false;
    sdt_header_t *sdt;
    bool validate_sdt(sdt_header_t* header) {
        uint64_t sum = 0;
        for (uint64_t i = 0; i < header->length; i++) {
            sum += *(uint8_t*)((uint64_t)header+i);
        }
        return (sum % 0x100) == 0;
    }
    
    void init_acpi() {
        print("Initializing ACPI\n\r");
        RSDP_t *rsdp = get_rsdp();
        if (!rsdp) { print("Could not find RSDP"); asm("cli"); for (;;); }
        if (!check_rsdp(rsdp)) { print("Invalid RSDP"); asm("cli"); for (;;); }
        printf("Revision: %x\n\rChecksum: %x\n\rOEM: %c%c%c%c%c%c\n\r", rsdp->revision, rsdp->checksum, rsdp->oemid[0], rsdp->oemid[1], rsdp->oemid[2], rsdp->oemid[3], rsdp->oemid[4], rsdp->oemid[5]);
        if (!check_version(rsdp)) {
            print("Loading RSDT...");
            sdt = (sdt_header_t*)rsdp->rsdt;
            print("Done\n\r");
            if (!validate_sdt(sdt)) { print("Invalid RSDT."); asm("cli"); for(;;); }
            return;
        }
        print("ACPI >= 2.0 supported\n\r");
        RSDP20_t *rsdp20 = (RSDP20_t*)rsdp;
        extended = true;
        if (!check_rsdp(rsdp20)) { print("Invalid RSDP 2.0"); asm("cli"); for (;;); }
        print("Loading XSTD...");
        sdt = (sdt_header_t*)rsdp20->xsdt;
        print("Done\n\r");
        if (!validate_sdt(sdt)) { print("Invalid XSDT."); asm("cli"); for(;;); }
    }

    void print_sdt(sdt_header_t* header) {
        printf("ID: %c%c%c%c\n\rLength: %x\n\rRevision: %x\n\rChecksum: %x\n\rOEM ID: %c%c%c%c%c%c\n\rOEM Table ID: %c%c%c%c%c%c%c%c\n\rOEM Revision: %x\n\r",
            header->signature[0],header->signature[1],header->signature[2],header->signature[3],
            header->length,
            header->revision,
            header->checksum,
            header->oem_id[0],header->oem_id[1],header->oem_id[2],header->oem_id[3],header->oem_id[4],header->oem_id[5],
            header->oem_table_id[0],header->oem_table_id[1],header->oem_table_id[2],header->oem_table_id[3],header->oem_table_id[4],header->oem_table_id[5],header->oem_table_id[6],header->oem_table_id[7],
            header->oem_revision
        );
    }

    void print_address(acpi_address_t addr) {
        print("Address Space: ");
        switch (addr.address_space) {
            case ACPI_ADDRESS_SPACE_RAM:
                print("RAM");
                break;
            case ACPI_ADDRESS_SPACE_PORT:
                print("PORT");
                break;
            default:
                print("?");
                break;
        }
        printf("\n\rBit Width: %x\n\rBit Offset: %x\n\rAccess Size: ",addr.bit_width,addr.bit_offset);
        switch (addr.access_size) {
            case 0:
                print("Undefined");
                break;
            case 1:
                print("Byte");
                break;
            case 2:
                print("Word");
                break;
            case 3:
                print("Double Word");
                break;
            case 4:
                print("Quad Word");
                break;
        }
        printf("\n\rAddress: %h",addr.address);
    }

    sdt_header_t* get_table(char* signature) {
        if (extended) {
            return get_table((xsdt_t*)sdt,signature);
        }
        return get_table((rsdt_t*)sdt,signature);
    }

    sdt_header_t* get_table(sdt_header_t* header, uint8_t entry_size, uint64_t entries, char* signature) {
        sdt_header* table = 0;
        for (uint64_t i = 0; i < entries; i++) {
            uint64_t new_addr = *(uint64_t*)((uint64_t)header + sizeof(sdt_header_t) + i*entry_size);
            if (entry_size == 4) new_addr = (new_addr & 0xffffffff);
            sdt_header_t* new_table = (sdt_header_t*)new_addr;
            debugf("%h: %c%c%c%c\n\r",new_table,new_table->signature[0],new_table->signature[1],new_table->signature[2],new_table->signature[3]);
            if (memcmp(new_table->signature,signature,4)) table = new_table;
        }
        if (!table) { printf("No table with signature %s.", signature); asm("cli"); for (;;); }
        if (!validate_sdt(table)) { print("Invalid SDT."); asm("cli"); for(;;); }
        return table;
    }

    sdt_header_t* get_table(rsdt_t* rsdt, char* signature) {
        uint8_t entry_size = 4;
        uint64_t entries = (rsdt->header.length - sizeof(sdt_header_t)) / entry_size;
        return get_table(&rsdt->header, entry_size, entries, signature);
    }

    sdt_header_t* get_table(xsdt_t* xsdt, char* signature) {
        uint8_t entry_size = 8;
        uint64_t entries = (xsdt->header.length - sizeof(sdt_header_t)) / entry_size;
        return get_table(&xsdt->header, entry_size, entries, signature);
    }

    bool check_rsdp(RSDP_t* rsdp) {
        uint64_t sum = 0;
        for (uint8_t i = sizeof(RSDP_t); i < 0; i++) {
            sum += *(uint8_t*)((uint64_t)rsdp + i);
        }
        return !(sum&0xF);
    }

    bool check_rsdp(RSDP20_t* rsdp) {
        uint64_t sum = 0;
        for (uint8_t i = (sizeof(RSDP20_t)-sizeof(RSDP_t)); i < 0; i++) {
            sum += *(uint8_t*)((uint64_t)&rsdp->length + i);
        }
        return !(sum&0xF);
    }

    bool check_version(RSDP_t* rsdp) {
        return rsdp->revision == 2;
    }

    RSDP_t* get_rsdp() {
        RSDP_t *rsdp = (RSDP_t*)0;
        for (uint64_t i=0; i<8192*2; i++) {
            RSDP_t *new_rsdp = (RSDP_t*)(0x80000 + 0x10 * i);
            if (memcmp(new_rsdp->signature,"RSD PTR ",8)) {
                rsdp = new_rsdp;
            }
        }
        for (uint64_t i=0; i<8192*2; i++) {
            RSDP_t *new_rsdp = (RSDP_t*)(0xE0000 + 0x10 * i);
            if (memcmp(new_rsdp->signature,"RSD PTR ",8)) {
                rsdp = new_rsdp;
            }
        }
        return rsdp;
    }
}