#ifndef  _CONFIG_H_
#define _CONFIG_H_

/*
 * �˿ڷ���
 */
// P0 : 0...8: 4x8 segment select
// P1 :	0...8: 4x4 keyboard 
// P2 ��0...4: 4x8 block select, 4 is the vled
//      5:current AD   6:   7:PWM driver
// P3 :	serial/INT..


#include <AT89X52.H>

#include <delay.h>

#include <4x4keyboard.h>

#include <4x8seg_vled.h>

#endif