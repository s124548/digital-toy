#ifndef _AVR_IO_
#define _AVR_IO_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "bitops.h"


/*for AVR */
/*
    #define _MMIO_BYTE(mem_addr) (*(volatile uint8_t *)(mem_addr))

    #define _SFR_IO8(io_addr) _MMIO_BYTE((io_addr) + 0x20)

   * PIN/DDR/PORTD address is sequnce 

	#define PIND	_SFR_IO8(0x10)
	#define DDRD	_SFR_IO8(0x11)
	#define PORTD	_SFR_IO8(0x12)
*/

#define INPUT   (char)0
#define OUTPUT  (char)1
void __set_port_mode(volatile unsigned char* port_addr, char mode, char n, char m);


/*i.e.  _pins_mode(PORTD,3,INPUT) */
#define _pin_mode(PORT,pin,mode) __set_port_mode(&PORT,mode,pin,pin)
#define _pins_mode(PORT,n,m,mode) __set_port_mode(&PORT,mode,n,m)



#define _inb(port) \
 		(*(volatile uint8_t *)(&(port)-2) ) 
#define _outb(port,val) \
 		(*(volatile uint8_t *)(&port)  = val;
			
#define barrier() __asm__ __volatile__("": : :"memory")

#define cli()   __asm__ __volatile__("CLI");
#define sti()   __asm__ __volatile__("SEI");





#endif
