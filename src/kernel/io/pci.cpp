#include <pci.h>
#include <vga.h>
#include <io.h>

namespace PCI {
    char* device_class_names[] = {
        "Unclassified",
        "Mass Storage Controller",
        "Network Controller",
        "Display Controller",
        "Multimedia Controller",
        "Memory Controller",
        "Bridge Device",
        "Simple Communication Controller",
        "Base System Peripheral",
        "Input Device Controller",
        "Docking Station", 
        "Processor",
        "Serial Bus Controller",
        "Wireless Controller",
        "Intelligent Controller",
        "Satellite Communication Controller",
        "Encryption Controller",
        "Signal Processing Controller",
        "Processing Accelerator",
        "Non Essential Instrumentation"
    };
    uint16_t read_config_word(uint8_t bus,uint8_t device, uint8_t func, uint8_t offset) {
        uint32_t addr;
        uint32_t lbus  = (uint32_t)bus;
        uint32_t lslot = (uint32_t)device;
        uint32_t lfunc = (uint32_t)func;
        uint16_t tmp = 0;

        addr = (uint32_t)((lbus << 16) | (lslot << 11) |
                  (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

        outl(PCI_CONFIG_ADDRESS, addr);
        uint32_t data = inl(PCI_CONFIG_DATA);
        uint16_t data015 = (uint16_t)data;
        //data015 = ((uint16_t)(data015 >> 8) | (uint16_t)(data015 << 8));
        uint16_t data1531 = (uint16_t)(data >> 16);
        //data1531 = ((uint16_t)(data1531 >> 8) | (uint16_t)(data1531 << 8));
        data = (data015<<16) | (data1531<<0);
        tmp = (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
        return tmp;
    }

    void print_pci(void) {
        uint8_t function;
        uint8_t bus;
        uint16_t headerType = read_config_word(0, 0, 0, 0xC);
        if ((headerType & 0x80) == 0) {
            print_bus(0);
        } else {
            for (function = 0; function < 8; function++) {
                if (read_config_word(0, 0, function,0x2) != 0xFFFF) break;
                bus = function;
                print_bus(bus);
            }
        }
    }

    void print_bus(uint8_t bus) {
        uint8_t device;
        for (device = 0; device < 32; device++) {
            print_device(bus, device);
        }
    }

    void print_device(uint8_t bus, uint8_t device) {
        uint8_t function;
        for (function = 0; function < 8; function++) {
            print_function(bus, device, function);
        }
    }

    void print_vendor(uint8_t bus, uint8_t device, uint8_t func) {
        uint16_t vendor = read_config_word(bus,device,func,0x2);
        switch (vendor) {
            case 0x8086:
                print("Intel Corp");
                break;
            case 0x1022:
                print("AMD");
                break;
            case 0x10DE:
                print("NVIDIA Corporation");
                break;
            case 0x1234:
                print("BOCHS");
                break;
            default:
                print("Unknown");//print_hex(vendor);
        }
    }

    void print_device_name(uint8_t bus, uint8_t device, uint8_t func) {
        uint16_t deviceID = read_config_word(bus,device,func,0x0);
        uint16_t vendor = read_config_word(bus,device,func,0x2);
        switch (vendor) {
            case 0x8086:
                print("Intel: ");
                switch (deviceID) {
                    case 0x29C0:
                        print("Express DRAM Controller");
                        break;
                    case 0x2918:
                        print("LPC Interface Controller");
                        break;
                    case 0x2922:
                        print("6 port SATA Controller [AHCI mode]");
                        break;
                    case 0x2930:
                        print("SMBus Controller");
                        break;
                    default:
                        print_hex(deviceID);
                }
                break;
            case 0x1234:
                print("VM: ");
                switch (deviceID) {
                    case 0x1111:
                        print("Video Display");
                        break;
                    default:
                        print_hex(deviceID);
                }
                break;
            case 0x10de:
                print("VM: ");
                switch (deviceID) {
                    case 0x0a20:
                        print("Video Display");
                        break;
                    default:
                        print_hex(deviceID);
                }
                break;
            default:
                print_hex(deviceID);
        }
    }
    
    char* ide_prog_if[] {
        "ISA Compatibillity mode-only",
        "PCI native mode-only",
        "ISA Compatibillity mode (support PCI native mode)",
        "PCI native mode (support ISA Compatibillity mode)"
    };

    char* network_controller[] {
        "Ethernet",
        "Token Ring",
        "FDDI",
        "ATM",
        "ISDN",
        "WorldFlip",
        "PICMG 2.14 Multi Computing",
        "InfiniBand",
        "Fabric"
    };

    void print_device_type(uint8_t dclass, uint8_t dsubclass, uint8_t prog_if) {
        switch (dclass) {
            case 0x6: // Bridge
                switch (dsubclass) {
                    case 0x0:print("Host Bridge");break;
                    case 0x1:print("ISA Bridge");break;
                    case 0x2:print("EISA Bridge");break;
                    case 0x3:print("MCA Bridge");break;
                    case 0x4:print("PCI-to-PCI Bridge");break;
                    case 0x5:print("PCMCIA Bridge");break;
                    case 0x6:print("NuBus Bridge");break;
                    case 0x7:print("CardBus Bridge");break;
                    case 0x8:print("RACEway Bridge");break;
                    case 0x9:print("PCI-to-PCI Bridge");break;
                    case 0xA:print("InfiniBand-to-PCI Bridge");break;
                    case 0x80:print("Other Bridge");break;
                    default:print("Bridge");
                }
                break;
            case 0x3: // Display
                switch (dsubclass) {
                    case 0x0:
                        switch (prog_if) {
                            case 0x00:print("VGA");break;
                            case 0x01:print("8514-Compatible");break;
                        }
                        print(" Display Controller");break;
                    case 0x1:print("XGA Display Controller");break;
                    case 0x2:print("3D Display Controller");break;
                    case 0x80:print("Other Display Controller");break;
                    default:print("Display Controller");
                }
                break;
            case 0x1: // Mass Storage
                switch (dsubclass) {
                    case 0x0:print("SCSI Bus Controller");break;
                    case 0x1:printf("IDE Controller (%s) %s",ide_prog_if[(prog_if&0x7f)/5],prog_if & 0x80 ? "supports bus mastering" : "");break;
                    case 0x2:print("Floppy Disk Controller");break;
                    case 0x3:print("IPI Bus Controller");break;
                    case 0x4:print("RAID Controller");break;
                    case 0x5:print("ATA Controller");break;
                    case 0x6:print("SATA Controller");break;
                    case 0x7:print("Serial Attached SCSI Controller");break;
                    case 0x8:print("Non-Volatile Memory Controller");break;
                    case 0x80:print("Other Display Controller");break;
                    default:print("Display Controller");
                }
                break;
            case 0xC: // Serial
                switch (dsubclass) {
                    case 0x0: print("FireWire Controller"); break;
                    case 0x1: print("Access Bus Controller"); break;
                    case 0x2: print("SSA"); break;
                    case 0x3: print("USB Controller ");
                        switch (prog_if) {
                            case 0x00: print("(UHCI)"); break;
                            case 0x10: print("(OHCI)"); break;
                            case 0x20: print("(2.0)"); break;
                            case 0x30: print("(3.0)"); break;
                            case 0x80: print("(Unspecified)"); break;
                            case 0xFE: print("(Device)"); break;
                        }
                        break;
                    case 0x4: print("Fibre Channel"); break;
                    case 0x5: print("SMBus Controller"); break;
                    case 0x6: print("InfiniBand Controller"); break;
                    case 0x7: print("IPMI Interface"); break;
                    case 0x8: print("SERCOS Interface"); break;
                    case 0x9: print("CANbus Controller"); break;
                    case 0x80: print("Other Serial Bus Controller"); break;
                }
                break;
            case 0x2: // Network
                if (dsubclass == 0x80) print("Other");
                else print(network_controller[dsubclass]);
                print(" Network Controller");
                break;
            case 0x4:
                switch (dsubclass) {
                    case 0x00: print("Multimedia Video Controller"); break;
                    case 0x01: print("Multimedia Audio Controller"); break;
                    case 0x02: print("Computer Telephony Device"); break;
                    case 0x03: print("Audio Device"); break;
                    default: print("Other Multimedia Controller");
                }
                break;
            default:
                print(device_class_names[dclass]);
        }
    }

    void print_function(uint8_t bus, uint8_t device, uint8_t func) {
        if (read_config_word(bus,device,func,0x2) == 0xFFFF) return;
        if (read_config_word(bus,device,func,0x0) == 0x0000) return;
        if (read_config_word(bus,device,func,0x0) == 0xFFFF) return;
        print_vendor(bus,device,func);
        print(" / ");
        uint16_t dclass_specification = read_config_word(bus,device,func,0x8);
        uint8_t dclass = dclass_specification >> 8;
        uint8_t dsubclass = dclass_specification & 0xFF;
        uint8_t prog_if = (read_config_word(bus,device,func,0xa) & 0xff00) >> 8;
        print_device_type(dclass,dsubclass,prog_if);
        //print(device_class_names[dclass]);
        print("\n\r");
    }
}