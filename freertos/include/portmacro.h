/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * portmacro.h  : 数据类型重定义
 *----------------------------------------------------------------*/
#ifndef __PORTMACRO__H
#define __PORTMACRO__H
#include "stdint.h"
#include "stddef.h"
/*----------------------------------------------------------------
 *数据类型重定义 
 *----------------------------------------------------------------*/
#define port_CHAR		char
#define port_FLOAT		float
#define port_DOUBLE		double
#define port_LONG		long
#define port_SHORT		short
#define port_STACK_TYPE	unsigned int
#define port_BASE_TYPE	long

typedef port_STACK_TYPE Stack_Type_t;
typedef long Base_Type_t;
typedef unsigned long U_Base_Type_t;

typedef port_STACK_TYPE* Stack_Type_p;
typedef long* Base_Type_p;
typedef unsigned long* U_Base_Type_p;

/* config_USE_16_BIT_TICKS 配置使用16位或者32位，本项目代码使用32位*/
#if( config_USE_16_BIT_TICKS == 1 )
	typedef uint16_t Tick_Type_t;
	#define port_MAX_DELAY ( Tick_Type_t ) 0xffff
#else
	typedef uint32_t Tick_Type_t;
	#define port_MAX_DELAY ( Tick_Type_t ) 0xffffffffUL
#endif

/* 中断控制状态寄存器：0xe000ed04
 * Bit 28 PENDSVSET: PendSV 悬起位
 */
#define port_NVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define port_NVIC_PENDSVSET_BIT		( 1UL << 28UL )

#define port_SY_FULL_READ_WRITE		( 15 )

#define port_YIELD()																\
{																				\
	/* 触发PendSV，产生上下文切换 */								                \
	port_NVIC_INT_CTRL_REG = port_NVIC_PENDSVSET_BIT;								\
	__dsb( port_SY_FULL_READ_WRITE );											\
	__isb( port_SY_FULL_READ_WRITE );											\
}
////////////////////////////////////////////////////////////
extern void v_Port_Enter_Critical( void );
extern void v_Port_Exit_Critical( void );

#define port_ENTER_CRITICAL()					v_Port_Enter_Critical()
#define port_EXIT_CRITICAL()					v_Port_Exit_Critical()

/* 临界区管理 */
#define port_DISABLE_INTERRUPTS()				v_Port_Raise_BASEPRI()//不带返回值，不能嵌套
#define port_SET_INTERRUPT_MASK_FROM_ISR()		ul_Port_Raise_BASEPRI()//带返回值，可以嵌套，中断中有中断


#define port_ENABLE_INTERRUPTS()					v_Port_Set_BASEPRI( 0 )// 不带中断保护的开中断函数
#define port_CLEAR_INTERRUPT_MASK_FROM_ISR(x)	v_Port_Set_BASEPRI(x)// 带中断保护的开中断函数

#define port_TASK_FUNCTION( v_Function, pvParameters ) void v_Function( void *pvParameters )
  

#define port_INLINE __inline

#ifndef port_FORCE_INLINE
	#define port_FORCE_INLINE __forceinline
#endif




#ifndef config_USE_PORT_OPTIMISED_TASK_SELECTION
	#define config_USE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if config_USE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* 检测优先级配置 */
	#if( config_MAX_PRIORITIES > 32 )
		#error config_USE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when config_MAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
	#endif

	/* 根据优先级设置/清除优先级位图中相应的位 */
	#define port_RECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
	#define port_RESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

	/*-----------------------------------------------------------*/

	#define port_GET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t ) __clz( ( uxReadyPriorities ) ) )

#endif /* taskRECORD_READY_PRIORITY */
       

static port_FORCE_INLINE void v_Port_Raise_BASEPRI( void )
{
uint32_t ulNewBASEPRI = config_MAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		msr basepri, ulNewBASEPRI 
		dsb 
		isb 
	}
}



static port_FORCE_INLINE void  v_Port_Set_BASEPRI( uint32_t ulBASEPRI )
{
	__asm
	{
		/* Barrier instructions are not used as this function is only used to
		lower the BASEPRI value. */
		msr basepri, ulBASEPRI
	}
}


static port_FORCE_INLINE void v_Port_Clear_BASEPRI_From_ISR( void )
{
	__asm{msr basepri, #0}
}

static port_FORCE_INLINE uint32_t ul_Port_Raise_BASEPRI( void )
{
uint32_t ulReturn, ulNewBASEPRI = config_MAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		mrs ulReturn, basepri
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}

	return ulReturn;
}




#endif // !__PORTMACRO__H
