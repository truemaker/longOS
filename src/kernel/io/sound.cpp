#include <sound.h>
#include <io.h>
#include <timer.h>
namespace PCSPK {
     //Play sound using built in speaker
    void play_sound(uint32_t nFrequence) {
    	uint32_t Div;
    	uint8_t tmp;
    
           //Set the PIT to the desired frequency
    	Div = 1193180 / nFrequence;
    	outb(0x43, 0xb6);
    	outb(0x42, (uint8_t) (Div) );
    	outb(0x42, (uint8_t) (Div >> 8));
    
           //And play the sound using the PC speaker
    	tmp = inb(0x61);
     	if (tmp != (tmp | 3)) {
    		outb(0x61, tmp | 3);
    	}
    }
    
    //make it shutup
    void nosound(void) {
    	uint8_t tmp = inb(0x61) & 0xFC;
    	outb(0x61, tmp);
    }
    
    //Make a beep
    void beep(void) {
    	play_sound(1000);
    	PIT::sleep(20);
    	nosound();
    }
}