#ifndef _DESCRIPTION_H
#define _DESCRIPTION_H


//规定本程序涉及中断固定位于分组2，即有2位（0~4）主优先级（抢占优先级）和2位（4个）次优先级（响应优先级）
#define USER_INT_GROUP NVIC_PRIGROUP_PRE4_SUB0

//规定定时器中断主优先级为0，具有最高优先级，保证算法同步
#define USER_INT_PRIORITY_TIMER 0

//规定传感器中断主优先级为1，具有次高优先级
#define USER_INT_PRIORITY_SENSOR 1

//规定串口通讯中断主优先级为2，属于次低优先级
#define USER_INT_PRIORITY_UART 2

//规定其他中断主优先级为2，属于最低优先级
#define USER_INT_PRIORITY_OTHER 3




#endif

