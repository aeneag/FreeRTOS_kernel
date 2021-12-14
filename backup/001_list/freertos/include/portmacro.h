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

/* config_USE_16_BIT_TICKS 配置使用16位或者32位，本项目代码使用32位*/
#if( config_USE_16_BIT_TICKS == 1 )
	typedef uint16_t Tick_Type_t;
	#define port_MAX_DELAY ( Tick_Type_t ) 0xffff
#else
	typedef uint32_t Tick_Type_t;
	#define port_MAX_DELAY ( Tick_Type_t ) 0xffffffffUL
#endif

#endif // !__PORTMACRO__H
