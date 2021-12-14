/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * task.h       : 任务相关函数头文件
 *----------------------------------------------------------------*/
#ifndef __TASK_H
#define __TASK_H
#include "list.h"
#include "projdefs.h"
/*
*************************************************************************
*                               任务控制块TCB 
*************************************************************************
*/
typedef struct tsk_Task_Control_Block
{
	volatile Stack_Type_t    *pxTopOfStack;    /* 栈顶 */
	T_list_item		  		 xStateListItem;   /* 任务节点 */
    Stack_Type_t             *pxStack;         /* 任务栈起始地址 */	                                        
	char                     pcTaskName[ config_MAX_TASK_NAME_LEN ];    /* 任务名称，字符串形式 */
    Tick_Type_t              xTicksToDelay; /* 用于延时 */ 
	U_Base_Type_t			 uxPriority; //优先级成员
}T_tcb,*P_tcb;
/*
*************************************************************************
*                               宏定义 重定义
*************************************************************************
*/
typedef void * Task_Handle_t;/* 任务句柄 */

#define task_YIELD()			            port_YIELD()
#define task_IDLE_PRIORITY                  ( ( U_Base_Type_t ) 0U )
/****************************进入退出临界start**************************/
#define task_ENTER_CRITICAL()               port_ENTER_CRITICAL()
#define task_ENTER_CRITICAL_FROM_ISR()      port_SET_INTERRUPT_MASK_FROM_ISR()											

#define task_EXIT_CRITICAL()                port_EXIT_CRITICAL()
#define task_EXIT_CRITICAL_FROM_ISR( x )    port_CLEAR_INTERRUPT_MASK_FROM_ISR( x )	
/*******************************临界end*********************************/

/*
*************************************************************************
*                               函数声明 
*************************************************************************
*/
#if( config_SUPPORT_STATIC_ALLOCATION == 1 )
Task_Handle_t x_Task_Create_Static(	Task_Function_t pxTaskCode,          /* 任务入口 */
					            const char * const pcName,           /* 任务名称，字符串形式 */
					            const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
					            void * const pvParameters,           /* 任务形参 */
								U_Base_Type_t uxPriority,            /* 任务优先级，数值越大，优先级越高 */
					            Stack_Type_p const puxStackBuffer,   /* 任务栈起始地址 */
					            P_tcb const pxTaskBuffer );           /* 任务控制块指针 */
#endif /* config_SUPPORT_STATIC_ALLOCATION */

void prv_Initialise_Task_Lists( void );
void v_Task_Start_Scheduler( void );
void v_Task_Switch_Context( void ); 
void v_Task_Delay( const Tick_Type_t xTicksToDelay );
Base_Type_t x_Task_Increment_Tick( void );	


#endif // !__TASK_H

