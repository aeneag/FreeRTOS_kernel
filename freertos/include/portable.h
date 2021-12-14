/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * portable.h   : 
 *----------------------------------------------------------------*/
#ifndef __PORT_TABLE_H
#define __PORT_TABLE_H
#include "portmacro.h"
#include "projdefs.h"
Stack_Type_p px_Port_Initialise_Stack( Stack_Type_p pxTopOfStack, Task_Function_t pxCode, void *pvParameters );
Base_Type_t x_Port_Start_Scheduler( void );
#endif // !__PORT_TABLE_H


