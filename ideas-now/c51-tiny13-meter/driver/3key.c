
#include <config.h>
#include <3key.h>

		   

unsigned char key(unsigned char key)
{
    KEY_PORT= KEY_PORT|(KEY_ALL);  //set input
	
   if(0 == (KEY_PORT&key)){
      mdelay(5);
  	  
	  if(0==(KEY_PORT&key)) {
	     while(0==(KEY_PORT&key));
	     return   1;// !; // 1: pressed, 0: 
	  }
   }

   return 0;
}


