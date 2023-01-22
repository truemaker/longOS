#include <acpi.h>
#include <vga.h>
#include <memory.h>
#include <io.h>
#include <timer.h>

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

    void* get_s5() {
        fadt_t* fadt = (fadt_t*)get_table("FACP");
        sdt_header_t* dsdt = (sdt_header_t*)fadt->dsdt;
        if (!memcmp(dsdt->signature,"DSDT",4)) { print("Invalid DSDT.\n\r"); asm("cli"); for (;;); }
        char* s5_addr = (char*)((uint64_t)dsdt + 36);
        int dsdt_length = dsdt->length - 36;
        while (0 < dsdt_length--) {
            if (memcmp(s5_addr,"_S5_",4)) break;
            s5_addr++;
        }
        if (dsdt_length > 0) {
            return s5_addr;
        } else {
            print("\\_S5_ not present!");
            asm("cli");
            for (;;);
        }
        return 0;
    }

    uint16_t get_typa() {
        char* s5 = (char*)get_s5();
        if (!((s5[-1]==0x08) || ((s5[-2] == 0x08) && (s5[-1] == '\\'))  && (s5[4] == 0x12))) { print("\\_S5_ parse error."); asm("cli"); for (;;); }
        s5 += 5;
        s5 += ((*s5 & 0xC0) >> 6) + 2;
        if (*s5 == 0x0A) s5++;
        return *s5  << 10;
    }

    uint16_t get_typb() {
        char* s5 = (char*)get_s5();
        if (!((s5[-1]==0x08) || ((s5[-2] == 0x08) && (s5[-1] == '\\')) && (s5[4] == 0x12))) { print("\\_S5_ parse error."); asm("cli"); for (;;); }
        s5 += 5;
        s5 += ((*s5 & 0xC0) >> 6) + 2;
        if (*s5 == 0x0A) s5++;
        s5++;
        if (*s5 == 0x0A) s5++;
        return *s5  << 10;
    }

    void shutdown() {
        enable_acpi(); // Just to make sure
        fadt_t *fadt = (fadt_t*)get_table("FACP");
        uint16_t typa = get_typa();
        print("Powering off in 5 seconds...\n\r");
        PIT::sleep(5000);
        print("Sending TYPa...\n\r");
        outw(fadt->pm1a_control_block,get_typa() | (1<<13));
        print("Sending TYPb...\n\r");
        if (fadt->pm1b_control_block != 0) outw(fadt->pm1b_control_block,get_typb() | (1<<13));
    }

    void enable_acpi() {
        fadt_t *fadt = (fadt_t*)get_table("FACP");
        if ((inw(fadt->pm1a_control_block) & (1<<13)) == 0) {
            if (fadt->acpi_enable != 0 && fadt->smi_command_port != 0) {
                outb(fadt->smi_command_port,fadt->acpi_enable);
                uint64_t i = 0;
                while (inw(fadt->pm1a_control_block) & 1 == 0 && i < 300) {
                    PIT::sleep(10);
                    i++;
                }
                if (fadt->pm1b_control_block) {
                    while (inw(fadt->pm1b_control_block) & 1 == 0 && i < 300) {
                        PIT::sleep(10);
                        i++;
                    }
                }
                if (i < 300) {
                    print("ACPI enabled!\n\r");
                } else {
                    print("Couldn't enable ACPI");
                    asm("cli");
                    for (;;);
                }
            } else {
                print("Can't enable ACPI");
                asm("cli");
                for (;;);
            }
        } else {
            print("ACPI already enabled\n\r");
        }
    }

    bool is_hardware_table(sdt_header_t* header) {
        return memcmp(header->signature,"APIC",4) || memcmp(header->signature,"HPET",4);
    }

    void print_apic_table(sdt_header_t* header) {
        print("Found APIC table:\n\r");
        uint32_t loacal_apic_addr = *(uint32_t*)((uint64_t)header + 36 + 0);
        uint32_t flags = *(uint32_t*)((uint64_t)header + 36 + 4);
        printf("    Local APIC: %h\n\r    Legacy Dual PICs installed: %s\n\r    Mask all PIC Interrupts: %s\n\r    Entries:\n\r",loacal_apic_addr,(flags & 2)?"Yes":"No",(flags & 1)?"Yes":"No");
        uint64_t remaining = (header->length - 44);
        uint8_t* entry = (uint8_t*)((uint64_t)header + 44);
        while (remaining > 0) {
            print("        ");
            switch (entry[0]) {
                case 0: printf("Processor Local APIC                    : CPU %x APIC %x",entry[2],entry[3]); break;
                case 1: printf("I/O APIC                                : ID %x",entry[2]); break;
                case 2: printf("IO/APIC Source Override                 : BUS %x IRQ %x GSI %h",entry[2],entry[3],*(uint64_t*)&entry[4]); break;
                case 3: printf("IO/APIC Non-maskable interrupt source"); break;
                case 4: printf("Local APIC Non-maskable Interrupts      : CPU %x", entry[2]); break;
                case 5: printf("Local APIC Address Override"); break;
                case 9: printf("Processor Local x2APIC"); break;
            }
            print("\n\r");
            //printf("        Entry Type: %x\n\r        Length: %x\n\r",entry[0],entry[1]);
            remaining -= entry[1];
            entry += entry[1];
        }
    }
    
    void print_hpet_table(sdt_header_t* header) {
        print("Found APIC table:\n\r");
        printf("    HPET Number: %x\n\r", *(uint8_t*)((uint64_t)header + 36 + 16));
    }

    void print_hardware_table(sdt_header_t* header) {
        if (memcmp(header->signature,"APIC",4)) print_apic_table(header);
        if (memcmp(header->signature,"HPET",4)) print_hpet_table(header);
    }

    void detect_hardware() {
        sdt_header_t* header = sdt;
        uint64_t entry_size = extended ? 8 : 4;
        uint64_t entries = header->length - 36;
        for (uint64_t i = 0; i < entries; i++) {
            uint64_t new_addr = *(uint64_t*)((uint64_t)header + sizeof(sdt_header_t) + i*entry_size);
            if (entry_size == 4) new_addr = (new_addr & 0xffffffff);
            sdt_header_t* new_table = (sdt_header_t*)new_addr;
            if (!validate_sdt(new_table)) { print("Invalid SDT."); asm("cli"); for(;;); }
            if (is_hardware_table(new_table)) print_hardware_table(new_table);
        }
    }
}