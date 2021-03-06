#include "include/avrio.h"
#include "charger.h"
#include "pwm.h"


#define NICD_DETECT_PWM  3 /*only for detect cell*/
#define NICD_FAST_VOLTAGE 1.25 /*only start fast charging when cell voltage > NICD_FAST_VOLTAGE*/
/*
   PRE charging: single cell< 1.3V
 */
#define NICD_PRE_CHAGE_MA    0.350  /* 350mA */
#define V9_PRE_CHAGE_MA      0.050  /* 30mA*/

#define NICD_FAST_CHAGE_MA   1.800  /* 1500mA */
#define V9_FAST_CHAGE_MA     0.180  /* 100mA*/   

/*
   detect cell -exist--> detect type ------------------->  PRE charge -> FAST(detect/ir) ->TRICKLE -->STOP
									  |					    |
									  |--> DETECT_ARRAY ----> 
*/						             
void detect_cell(i_charger *ic);
void detect_type(i_charger *ic);
void detect_array(i_charger *ic);
void stop(i_charger *ic);
void charging_mode(i_charger *ic);

#define refV  ((float)5.1)

#define min(a,b)  ((a)<(b)?(a):(b))
#define max(a,b)  ((a)>(b)?(a):(b))

#define FILTER_PASS  10
float lowpass_adc(char ch)
{

#if 0
   float adc[FILTER_PASS];
   float low=_adc(ch);
   float top = low;
   float avg;
   unsigned char topi=0,lowi=0,i;

   for(i=0; i<FILTER_PASS; i++){
      adc[i] =  _adc(ch);
      if(adc[i] > top){
	     topi=i;
		 top = adc[i];
	  }   
      if(adc[i] <low ){
	     lowi=i;
		 low = adc[i];
	  }   
   }
   adc[topi] = 0;
   adc[lowi] = 0;

   low=0;
   for(i=0; i<FILTER_PASS; i++){
      low+=adc[i];
   }

   avg = low/(FILTER_PASS-2);
   if(topi==lowi)
       avg=low/(FILTER_PASS-1);

   return avg;
#endif 

   static float  last[7]={0,0,0.0,0,0,0}, low,delta;
   low = _adc(ch);
   delta = low-last[ch];
   if(delta<0) delta = -delta;

   	  
   if(delta>10){
      last[ch]= low*0.98+last[ch]*0.02;
   }else{
      last[ch] = last[ch]*0.98+low*0.02;
   } 

    return last[ch];
   
}
float adc_V()
{

//  sprintf(str,"%04imV",_adc(0));
   float adc= lowpass_adc(0);
   return (((float)adc)/1023)*refV*2.4; //5mV*2.4 =12mV 

}

float adc_A()
{

	float tmp=0.0;

 // sprintf(str,"%04imA",_adc(2));
    float adc= lowpass_adc(2);
  
   	tmp= ((float)adc/1023)*refV;
	tmp = tmp/15;
	return tmp = tmp/0.10; 

}

void info(char *e)
{
   lcd_cursor(8,0);
   lcd_puts(e);
}
void infon(char *e,char n)
{
   lcd_cursor(8,0);
   lcd_puts(e);
   lcd_putc(hex2c(n));
}
void ic_update_lcd(i_charger *ic)
{
   //first line of 1602 given to charger...
   lcd_cursor(0,0);


   if(ic->i_stage == STOP){
      lcd_puts("END");
      short x = 1000*ic->voltage;
	  print10(x); 
	  info("TOP");
	  x =1000*ic->top_voltage; 
	  print10(x);
  
   }else{
      lcd_puts("PWM");
      print10(pwm_getduty());
      lcd_puts(" ");
   }
      
}


void init_ic(i_charger *ic)
{
      ic->i_stage = DETECT_CELL;
	  ic->charging_mode = NOT_IN;
  	  ic->cell_type = Unknown;
	  ic->cell_pack = 0;  //start pack
      
	  ic->current = ic->voltage = 0;
	  ic->top_voltage = 0;
	  ic->delta_times = 0;

	  ic->charging_Amp = 0;
	  ic->ir = 0;
	  ic->abs_voltage = 0;
	  /*stop pwm*/
	  pwm_setduty(0);
}
void in_chaging(i_charger *ic)
{
    return;
}
void adj_current(i_charger *ic)
{
	unsigned char pwm= pwm_getduty();
	/*adjust to around the ma, and don't change again to detect -DeltaV*/
    char n=15+15;
	char step = 15;
	while(--n){
		if(adc_A() < (ic->charging_Amp-0.010)){
			pwm +=step;
			if(pwm>200)
				pwm=200;
 			pwm_setduty(pwm);
		}
		if(adc_A() > (ic->charging_Amp+0.010)){
		    step = 1;
			pwm -- ;
			if(!pwm)
				pwm = 1;
			pwm_setduty(pwm);
			

		}
       ic_update_lcd(ic);
       
	}
}


/*
	 DETECT_CELL,
	 DETECT_TYPE,
	 DETECT_ARRAY,
	 CHARCHING,
	 STOP,
*/
void (*_fun[STOP+1])(i_charger*)=
{
	detect_cell,
	detect_type,
	detect_array,
    in_chaging, /*in charging*/
	stop,
};

void charging(i_charger *ic)
{

    ic->voltage = adc_V();
    ic->current = adc_A();
	
	charging_mode(ic);

	void (*f)(i_charger*);
	f= _fun[ic->i_stage];
	f(ic);
}


void charging_mode(i_charger *ic)
{
/*
	enum  charger_stage i_stage;
	enum Cell_type cell_type;
	unsigned char  cell_pack; 
	
	
	float current;
	float voltage;
    float top_voltage; 
	char delta_times;

	float charging_Amp;  
	float ir;  
	float abs_voltage;
*/
   float deltaV = 0.010;

   if( (ic->voltage <= 0.100 /*V*/) || (ic->voltage >= 11.90) ){  //cell pull out
      init_ic(ic);

   }


   if(ic->i_stage != CHARCHING) /*not start charging*/
   		return;



   if( ic->abs_voltage < (NICD_FAST_VOLTAGE*ic->cell_pack)){
       ic->charging_Amp = NICD_PRE_CHAGE_MA;
	
	   if(ic->cell_pack == 7)
   		{
      		ic->charging_Amp = V9_PRE_CHAGE_MA;
   		}
   		ic->charging_mode = PRE;
	
		ic->abs_voltage = adc_V() - 0.15*ic->charging_Amp; //assume Ir is 0.15R

		   
   }else if(ic->charging_mode !=  FAST) {
       
	   ic->charging_Amp = NICD_FAST_CHAGE_MA;
	   if(ic->cell_pack == 7)
   	   {
       		ic->charging_Amp = V9_FAST_CHAGE_MA;
   	   }
       ic->charging_mode =  FAST;
	   adj_current(ic);
	      
   }
   

   

   /*adjust current if need*/
   if(ic->charging_mode !=  FAST)
   if( (ic->current>(ic->charging_Amp+0.03)) ||
       (ic->current<(ic->charging_Amp-0.03))
	 )
	  adj_current(ic);

   /* -DeltaV detect*/
   if(ic->charging_mode != FAST)
   		return;
   if(ic->voltage > ic->top_voltage){
       ic->top_voltage = ic->voltage;
	   info("TOP");
	   short x =1000*ic->top_voltage; 
	   print10(x);
	   ic->delta_times=0;
   }

   if (ic->cell_pack >2)
       deltaV = 0.015; /*reduce -dV, that's cell pack, because not all cell reach full at same time,
	                   so, -dv became small..*/
   if( ic->top_voltage > (ic->voltage+deltaV) ){
       	ic->delta_times++;
 		infon("   -dV:",ic->delta_times);
		delay(HZ/4);
		if(ic->delta_times >3){
			ic->i_stage = STOP; /*STOP and update abs voltage*/
			pwm_setduty(0);
		}
   }

}


/*************************************************/

#define BAT_LOW_V           (float)1.00    /* 0.5V  */

void detect_cell(i_charger *ic)
{
   float pwm_off,pwm_on,dv;
   
   pwm_setduty(0);
   delay(HZ/2);

   pwm_off = adc_V();
   pwm_setduty(10);
   delay(HZ/2);
   pwm_on = adc_V();
   pwm_setduty(0);
   
   if(pwm_off> pwm_on)
      dv = pwm_off-pwm_on;
   else 
      dv = pwm_on-pwm_off;

   if( (dv > (float)2.0) || (ic->voltage >= 12.00 ) || (ic->voltage <0.100)){
	  infon("NO CELL", (char)dv);
      return ;
   }
   else {
	  info("Plug IN ");
      delay(HZ/2);
   }
   if( ic->voltage < BAT_LOW_V ){
       pwm_setduty(NICD_DETECT_PWM); /*very pre charging*/
   }else {
       ic->i_stage = DETECT_TYPE;
	   ic->abs_voltage = ic->voltage;
   }
      
}


/* NOW: only NiCd cell-array V9PACK */
/*
           low   high 
  single   1.0   1.42
  2-array             2.0    2.84
  Li                  2.0                 4.2
  3-array                          3         4.26 
  4-array                               4            5.68
  5-array                                          5		    7.1
  6-array                                                 6           8.52
  7-array(V9-stack)                                            7             9.94        
 
*/
#define NICD_STOP_V     	1.42   /* 1.42V */
#define NICD_FAST_V 		1.00   /* 1.0V  */
#define NICD_MAX_V     	    1.50   /* 1.50V */
#define NICD_ARRAY_MIN_V	2.00   /*       */
void detect_type(i_charger *ic)
{
   
   if( ic->voltage < 1.0){ //very pre charge
      pwm_setduty(1);  
	  return;
   }
   if( ic->voltage < NICD_MAX_V){ 
      ic->cell_type =  NiCd;
	  ic->cell_pack = 1;  //single NiCd
      ic->i_stage = CHARCHING;
	  info("Single   ");
	  return;
   }

   if( ic->voltage >= NICD_ARRAY_MIN_V  ){
	  ic->cell_type = CELL_ARRAY;
	  ic->cell_pack = 2;  //start pack
	  ic->i_stage = DETECT_ARRAY;
	  return;

   }
 	   
}

/*
           low   high 
  single   1.0   1.42
  2-array             2.0    2.84
  3-array                           3       4.26
  4-array                               4            5.68
  7-array(V9-stack)                                            7             9.94        
 
*/
void detect_array(i_charger *ic)
{

   if( ic->voltage >= 7.00  ){
		ic->cell_pack = 7; 
		goto out;
   }
   	   
   if( ic->voltage >= 4.00 ){
	    ic->cell_pack = 4;  
	  	  goto out;;
   }	   

   if( ic->voltage >= 2.00 ){
	  ic->cell_pack = 2;  
	  	 goto out; 
   }	

   /* <2.0, Err, 判定type的时候能保证电压>2.0 */
   if( ic->voltage < 2.00){
      init_ic(ic);
	  return;
   }
out:
   ic->i_stage = CHARCHING;
   infon("Pack  ",ic->cell_pack);
   return;
}

void stop(i_charger *ic)
{

	pwm_setduty(0);
	ic->abs_voltage = adc_V();    
    if(ic->voltage < (1.38*ic->cell_pack)){ /*why there is a -dV?*/
	   ic->i_stage = CHARCHING; /*restart charging */
	   ic->top_voltage = ic->voltage = 0;
	   adj_current(ic);
	}
}

