/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * main.c       : 主函数
 *----------------------------------------------------------------*/
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
/*
*************************************************************************
*                              全局变量
*************************************************************************
*/
port_CHAR flag1;
port_CHAR flag2;
port_CHAR flag3;

extern T_tcb px_Ready_Tasks_Lists[ config_MAX_PRIORITIES ];
/*
*************************************************************************
*                        任务控制块 & STACK 
*************************************************************************
*/
Task_Handle_t Task1_Handle;
#define TASK1_STACK_SIZE                    128
Stack_Type_t Task1Stack[TASK1_STACK_SIZE];
T_tcb Task1TCB;

Task_Handle_t Task2_Handle;
#define TASK2_STACK_SIZE                    128
Stack_Type_t Task2Stack[TASK2_STACK_SIZE];
T_tcb Task2TCB;

Task_Handle_t Task3_Handle;
#define TASK3_STACK_SIZE                    128
Stack_Type_t Task3Stack[TASK3_STACK_SIZE];
T_tcb Task3TCB;

/*
*************************************************************************
*                               函数声明 
*************************************************************************
*/
void delay (uint32_t count);
void Task1_Entry( void *p_arg );
void Task2_Entry( void *p_arg );
void Task3_Entry( void *p_arg );
/*
************************************************************************
*                                main函数
************************************************************************
*/
int main(void)
{	
    /* 硬件初始化 */
	/* 将硬件相关的初始化放在这里，如果是软件仿真则没有相关初始化代码 */
    /* 创建任务 */
    Task1_Handle = x_Task_Create_Static( (Task_Function_t)Task1_Entry,   /* 任务入口 */
					                  (char *)"Task1",               /* 任务名称，字符串形式 */
					                  (uint32_t)TASK1_STACK_SIZE ,   /* 任务栈大小，单位为字 */
					                  (void *) NULL,                 /* 任务形参 */
                                      (U_Base_Type_t) 2,               /* 任务优先级，数值越大，优先级越高 */
					                  (Stack_Type_t *)Task1Stack,     /* 任务栈起始地址 */
					                  (P_tcb)&Task1TCB );          /* 任务控制块 */
                                
    Task2_Handle = x_Task_Create_Static( (Task_Function_t)Task2_Entry,   /* 任务入口 */
					                  (char *)"Task2",               /* 任务名称，字符串形式 */
					                  (uint32_t)TASK2_STACK_SIZE ,   /* 任务栈大小，单位为字 */
					                  (void *) NULL,                 /* 任务形参 */
                                      (U_Base_Type_t) 2,               /* 任务优先级，数值越大，优先级越高 */                                          
					                  (Stack_Type_t *)Task2Stack,     /* 任务栈起始地址 */
					                  (P_tcb)&Task2TCB );          /* 任务控制块 */
                                      
    Task3_Handle = x_Task_Create_Static( (Task_Function_t)Task3_Entry,   /* 任务入口 */
					                  (char *)"Task3",               /* 任务名称，字符串形式 */
					                  (uint32_t)TASK3_STACK_SIZE ,   /* 任务栈大小，单位为字 */
					                  (void *) NULL,                 /* 任务形参 */
                                      (U_Base_Type_t) 3,               /* 任务优先级，数值越大，优先级越高 */                                          
					                  (Stack_Type_t *)Task3Stack,     /* 任务栈起始地址 */
					                  (P_tcb)&Task3TCB );          /* 任务控制块 */                                      
                                      
    port_DISABLE_INTERRUPTS();
                                      
    /* 启动调度器，开始多任务调度，启动成功则不返回 */
    v_Task_Start_Scheduler();                                      
    
    for(;;);
}

/*
*************************************************************************
*                               函数实现
*************************************************************************
*/
/* 软件延时 */
void delay (uint32_t count)
{
	for(; count!=0; count--);
}
/* 任务1 */
void Task1_Entry( void *p_arg )
{
	for( ;; )
	{
		flag1 = 1;      
        delay (100);		
		flag1 = 0;
        delay (100);
	}
}

/* 任务2 */
void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		flag2 = 1;
        delay (100);		
		flag2 = 0;
        delay (100);
           
	}
}


void Task3_Entry( void *p_arg )
{
	for( ;; )
	{
		flag3 = 1;
        v_Task_Delay( 1 );
        //delay (100);		
		flag3 = 0;
        v_Task_Delay( 1 );
        //delay (100);
	}
}
/*
*************************************************************************
*                               空闲任务
*************************************************************************
*/
/* -------------------------获取空闲任务的内存-------------------------- */
Stack_Type_t IdleTaskStack[config_MINIMAL_STACK_SIZE];
T_tcb IdleTaskTCB;
void v_Application_Get_Idle_Task_Memory( T_tcb **ppxIdleTaskTCBBuffer, 
                                    Stack_Type_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack; 
		*pulIdleTaskStackSize=config_MINIMAL_STACK_SIZE;
}


