/*----------------------------------------------------------------
 * author           : Aen
 * blog			    : https://aeneag.xyz
 * date             : 2021/12/13
 * FreeRTConfig.h   : FreeRTOS配置文件
 *----------------------------------------------------------------*/
#ifndef __FREEROTS_CONFIG_H
#define __FREEROTS_CONFIG_H

#define config_USE_16_BIT_TICKS		            0
#define config_MAX_TASK_NAME_LEN		        ( 16 )
#define config_SUPPORT_STATIC_ALLOCATION        1
#define config_MAX_PRIORITIES		            ( 5 )

#define config_KERNEL_INTERRUPT_PRIORITY 		255   /* 高四位有效，即等于0xff，或者是15 */
#define config_MAX_SYSCALL_INTERRUPT_PRIORITY 	191   /* 高四位有效，即等于0xb0，或者是11 */

#define config_MINIMAL_STACK_SIZE	( ( unsigned short ) 128 )

#define config_CPU_CLOCK_HZ			( ( unsigned long ) 25000000 )	
#define config_TICK_RATE_HZ			( ( Tick_Type_t ) 1000 )

#define config_USE_PREEMPTION            1

#define x_Port_PendSV_Handler   PendSV_Handler
#define x_Port_Sys_Tick_Handler  SysTick_Handler
#define v_Port_SVC_Handler      SVC_Handler
#endif // !__FREEROTS_CONFIG_H
