
#include "taskschedule.h"
#include "computerFrameParse.h"
#include "clock.h"

static uint32_t	TimeTaskResumeSta[MAX_TIMETASK/32+1] = {0,};

extern EventStatus system_1s_task(void);
TIME_TASK MyTimerTask[MAX_TIMETASK] =
{

    {ENABLE, DISABLE, 0, 1, 	 usart_dispose_recvDataTask},

    {ENABLE, DISABLE, 20, 200,      system_1s_task}

};


/*******************************************************************************
* Function Name  : SetTimerTask
* Description    : Setup timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetTimerTask(TIMERTASK task, uint32_t time)
{
    MyTimerTask[task].TaskTimerState = ENABLE;
    MyTimerTask[task].TaskExTimer = time;
    MyTimerTask[task].TaskTimerCount = time + ins_tick_get();
}

/*******************************************************************************
* Function Name  : ClrTimerTask
* Description    : Cancel timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ClrTimerTask(TIMERTASK task)
{
    MyTimerTask[task].TaskTimerState = DISABLE;
}

/*******************************************************************************
* Function Name  : ResumeTimerTask
* Description    : resume timer task .
* Input          : task MAX_TIMETASK: resume all task otherwise resume appointed task
* Output         : None
* Return         : None
*******************************************************************************/
void ResumeTimerTask(TIMERTASK task)
{
    uint8_t i,j,k;
    if (task > MAX_TIMETASK)return;
    if (task == MAX_TIMETASK)
    {
        for(i = 0; i < MAX_TIMETASK; i++)
        {
            j = i/32;
            k = i%32;
            if(BIT_CHECK(TimeTaskResumeSta[j],k))
            {
                BIT_CLR(TimeTaskResumeSta[j],k);
                MyTimerTask[i].TaskTimerState = ENABLE;
            }
        }
    }
    else
    {
        MyTimerTask[task].TaskTimerState = ENABLE;
    }
}

/*******************************************************************************
* Function Name  : SuspendTimerTask
* Description    : suspend timer task .
* Input          : task MAX_TIMETASK: suspend all task otherwise suspend appointed task
* Output         : None
* Return         : None
*******************************************************************************/
void SuspendTimerTask(TIMERTASK task)
{
    uint8_t i,j,k;
    if (task > MAX_TIMETASK)return;
    if (task == MAX_TIMETASK)
    {
        for(i = 0; i < MAX_TIMETASK; i++)
        {
            if(MyTimerTask[i].TaskTimerState == ENABLE)
            {
                j = i/32;
                k = i%32;
                BIT_SET(TimeTaskResumeSta[j],k);
                MyTimerTask[i].TaskTimerState = DISABLE;
            }
        }
    }
    else
    {
        MyTimerTask[task].TaskTimerState = DISABLE;
    }
}

/*******************************************************************************
* Function Name  : TimerTaskScheduler
* Description    : Schedule Timer task .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TimerTaskScheduler(void)
{
    uint8_t i;
    uint32_t nowtimer;
    EventStatus validflag;
    nowtimer = ins_tick_get();

    for(i = 0; i < MAX_TIMETASK; i++)
    {
        if(MyTimerTask[i].TaskTimerState != ENABLE)
        {
            continue;
        }
        if(MyTimerTask[i].TaskOverflow == ENABLE)
        {
            if(nowtimer >= MyTimerTask[i].TaskTimerCount)
            {
                MyTimerTask[i].TaskTimerCount = nowtimer + MyTimerTask[i].TaskExTimer;
                validflag = MyTimerTask[i].operate();
                MyTimerTask[i].TaskTimerState = validflag;
                MyTimerTask[i].TaskOverflow = DISABLE;
            }
        }
        else
        {
            if(nowtimer >= MyTimerTask[i].TaskTimerCount)
            {
                MyTimerTask[i].TaskTimerCount = nowtimer + MyTimerTask[i].TaskExTimer;
                validflag = MyTimerTask[i].operate();
                MyTimerTask[i].TaskTimerState = validflag;
                if(MyTimerTask[i].TaskTimerCount < nowtimer)
                    MyTimerTask[i].TaskOverflow = ENABLE;

            }
        }
    }
}


