
#include "bitops.h"
#include <avr/io.h>
#include <util/delay.h>

#include "tiny-uc.h"


/* FAST PWM mode
The counter counts from BOTTOM to TOP then restarts from BOTTOM. 


---------------
TCCR0A :  COM0A1 COM0A0 

WGM2:0 
TOP is defined as 0xFF when WGM2:0 = 3  *
and OCR0A when WGM2:0 = 7

COM01 COM00: 
1       0     Clear OC0A on Compare Match, set OC0A at TOP *

------------------
TCCR0B :
WGM20  

CS02 CS01 CS00 ��
0     1      0       clkI/O/8 (From prescaler)


------------------
TCNT0  -

OCR0A �C Output Compare Register , cmpare to TCN0

GTCCR �C General Timer/Counter Control Register
             Bit              7            0
                            TSM          PSR10     
             Read/Write     R/W           R/W
             Initial Value    0             0
            Bit 7 �C TSM: Timer/Counter Synchronization Mode
            Writing the TSM bit to one activates the Timer/Counter Synchronization mode. In this mode, the
            value that is written to the PSR10 bit is kept, hence keeping the Prescaler Reset signal asserted.
            This ensures that the Timer/Counter is halted and can be configured without the risk of advanc-
            ing during configuration. When the TSM bit is written to zero, the PSR10 bit is cleared by
            hardware, and the Timer/Counter start counting.
            Bit 0 �C PSR10: Prescaler Reset Timer/Counter0
            When this bit is one, the Timer/Counter0 prescaler will be Reset. This bit is normally cleared
            immediately by hardware, except ifexcept )

*/



void pwm_init()
{
  TCCR0A  =  ( _bits8(0b11,WGM00,WGM02) ) |  ( _bits8(0b10, COM0B0,COM0B1) |  ( _bits8(0b10, COM0A0,COM0A1) ));  
  TCCR0B  =  _bits8(0b100, CS00,CS02);     
  TCNT0  = 0;
  OCR0A = 0 ;  /* duty = 0; */

  GTCCR = 1; /*reset prescaler*/

}


void pwm_setduty(unsigned char duty)
{
  OCR0B = OCR0A=duty;
}
