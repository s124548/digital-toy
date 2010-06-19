
#include  "config.h"
#include  "timer.h"

/*
 * timer0, 
 *   1) 1ms ticks
 *	 2) tickless driver 
 */

volatile unsigned  long jiffers=0;  //250us per jiffers





#define settimer0(us) TH0 = (65536 - (us))>>8 ; /* (655535-us)/256 ����!!!!*/  \
                      TL0 = (65536 - (us))&0xFF ; 

#define settimer1(us) TH1 = (65535 - (us))>>8 ; /* (655535-us)/256 ����!!!!*/  \
                      TL1 = (65535 - (us))&0xFF ; 
 


void timer0_init()
{

   /*  timer 0 in mode 1   */
   TMOD |= 0x2;   
      
   //settimer0(PWM_CYCLE);
   TH1 = TH0 = (256-PWM_CYCLE); //100us  15 step , 600Hz
   // enable timer 0 interrupt   
   ET0 = 1;   
   TR0 = 1; //enable timer

}

				 
/*
 * Timer0: 256us, display driven ticks
 */
/*�ж�1�� �Ĵ�����1 (��ͨ����Ĭ���üĴ�����0)*/



void timer0(void) interrupt 1 using 2   
{   

	pwm_1kHz();

#if 0
    if(PWM_PIN) { //need close PWM
   
   	   PWM_PIN = 0;
	   settimer0(PWM_CYCLE-on_duty);
     //  vledx0();

	 
     }else { //need open PWM
  
       PWM_PIN = 1;
	   settimer0(on_duty);//on_duty);
//	   vledx1();

     }  
#endif

}  


void timer1_init()
{
  /* timer0 init */
   /*  timer 0 in mode 2   */
   TMOD |= 0x10;   
   // set timer speed   
   settimer1(MS_CYCLE);   	//per 250us

  
   // enable timer 1 interrupt   
   ET1 = 1;   
   
   TR1 = 1; //enable timer

}

void timer1(void) interrupt 3 using 1   
{   
 	 ms_scan_segvled(); //ɨ����ʾ

	 
	 jiffers++;
	 settimer1(MS_CYCLE);
}



