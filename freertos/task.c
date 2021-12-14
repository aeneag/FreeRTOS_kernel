/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * task.c       : 任务相关函数
 *----------------------------------------------------------------*/
#include "task.h"

/*
*************************************************************************
*                               函数声明 
*************************************************************************
*/
/*-----------------------------创建新的任务---------------------------*/
static void prv_Initialise_New_Task( 	Task_Function_t pxTaskCode,              /* 任务入口 */
									const char * const pcName,              /* 任务名称，字符串形式 */
									const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
									void * const pvParameters,              /* 任务形参 */
								    U_Base_Type_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
									Task_Handle_t * const pxCreatedTask,     /* 任务句柄 */
									P_tcb pxNewTCB );                       /* 任务控制块指针 */
static void prv_Add_New_Task_To_Ready_List( P_tcb pxNewTCB ); 
static port_TASK_FUNCTION( prvIdleTask, pvParameters );
static void prv_Add_Current_Task_To_Delayed_List( Tick_Type_t xTicksToWait ); 
static void prv_Reset_Next_Task_Unblock_Time( void );  
/*
*************************************************************************
*                               宏定义 
*************************************************************************
*/
/* -------------------------将任务添加到就绪列表------------------ */                                    
#define prv_Add_Task_To_Ready_List( pxTCB )																   \
	task_RECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												   \
	v_List_Insert_End( &( px_Ready_Tasks_Lists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) ); \

									
/* 查找最高优先级的就绪任务：通用方法 */                                    
#if ( config_USE_PORT_OPTIMISED_TASK_SELECTION == 0 )
	/* uxTopReadyPriority 存的是就绪任务的最高优先级 */
	#define task_RECORD_READY_PRIORITY( uxPriority )														\
	{																									\
		if( ( uxPriority ) > uxTopReadyPriority )														\
		{																								\
			uxTopReadyPriority = ( uxPriority );														\
		}																								\
	} /* taskRECORD_READY_PRIORITY */

	/*-----------------------------------------------------------*/

	#define task_SELECT_HIGHEST_PRIORITY_TASK()															\
	{																									\
	U_Base_Type_t uxTopPriority = uxTopReadyPriority;														\
																										\
		/* 寻找包含就绪任务的最高优先级的队列 */                                                          \
		while( list_LIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )							\
		{																								\
			--uxTopPriority;																			\
		}																								\
																										\
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */							            \
		list_GET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );			\
		/* 更新uxTopReadyPriority */                                                                    \
		uxTopReadyPriority = uxTopPriority;																\
	} /* taskSELECT_HIGHEST_PRIORITY_TASK */

	/*-----------------------------------------------------------*/

	/* 这两个宏定义只有在选择优化方法时才用，这里定义为空 */
	#define task_RESET_READY_PRIORITY( uxPriority )
	#define port_RESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )
    
/* 查找最高优先级的就绪任务：根据处理器架构优化后的方法 */
#else /* config_USE_PORT_OPTIMISED_TASK_SELECTION */

	#define task_RECORD_READY_PRIORITY( uxPriority )	port_RECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )

	/*-----------------------------------------------------------*/

	#define task_SELECT_HIGHEST_PRIORITY_TASK()														    \
	{																								    \
	U_Base_Type_t uxTopPriority;																		    \
																									    \
		/* 寻找最高优先级 */								                            \
		port_GET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );								    \
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */                                       \
		list_GET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( px_Ready_Tasks_Lists[ uxTopPriority ] ) );		    \
	} /* task_SELECT_HIGHEST_PRIORITY_TASK() */

	/*-----------------------------------------------------------*/
#if 0
	#define task_RESET_READY_PRIORITY( uxPriority )														\
	{																									\
		if( list_CURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )	\
		{																								\
			port_RESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );							\
		}																								\
	}
#else
    #define task_RESET_READY_PRIORITY( uxPriority )											            \
    {																							        \
            port_RESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );					        \
    }
#endif
    
#endif /* config_USE_PORT_OPTIMISED_TASK_SELECTION */
		
/* 
 * 当系统时基计数器溢出的时候，延时列表 px_Delayed_Task_List 和
 * px_Overflow_Delayed_Task_List 要互相切换
 */
#define task_SWITCH_DELAYED_LISTS()\
{\
	P_list pxTemp;\
	pxTemp = px_Delayed_Task_List;\
	px_Delayed_Task_List = px_Overflow_Delayed_Task_List;\
	px_Overflow_Delayed_Task_List = pxTemp;\
	xNumOfOverflows++;\
	prv_Reset_Next_Task_Unblock_Time();\
}	
/*
*************************************************************************
*                               全局变量 
*************************************************************************
*/
/*---------------------------任务就绪列表--------------------------*/
T_list px_Ready_Tasks_Lists[ config_MAX_PRIORITIES ];
/* --------当前正在运行的任务的任务控制块指针，默认初始化为NULL------ */
P_tcb volatile pxCurrentTCB = NULL;

/* ----------------------全局任务计时器加一操作------------------- */
static volatile U_Base_Type_t uxCurrentNumberOfTasks 	= ( U_Base_Type_t ) 0U;
static Task_Handle_t xIdleTaskHandle					= NULL;
static volatile Tick_Type_t xTickCount 				    = ( Tick_Type_t ) 0U;
static volatile U_Base_Type_t uxTopReadyPriority 		= task_IDLE_PRIORITY;
static U_Base_Type_t uxTaskNumber 					    = ( U_Base_Type_t ) 0U;


static T_list x_Delayed_Task_List1;
static T_list x_Delayed_Task_List2;
static P_list volatile px_Delayed_Task_List;
static P_list volatile px_Overflow_Delayed_Task_List;

static volatile Tick_Type_t xNextTaskUnblockTime		= ( Tick_Type_t ) 0U;
static volatile Base_Type_t xNumOfOverflows 			= ( Base_Type_t ) 0;
/*
*************************************************************************
*                              函数 
*************************************************************************
*/
/*--------------------------静态任务创建函数------------------------*/
#if( config_SUPPORT_STATIC_ALLOCATION == 1 )
Task_Handle_t x_Task_Create_Static(	Task_Function_t pxTaskCode,      /* 任务入口 */
					            const char * const pcName,           /* 任务名称，字符串形式 */
					            const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
					            void * const pvParameters,           /* 任务形参 */
								U_Base_Type_t uxPriority,            /* 任务优先级，数值越大，优先级越高 */
					            Stack_Type_p const puxStackBuffer,   /* 任务栈起始地址 */
					            P_tcb const pxTaskBuffer )           /* 任务控制块指针 */
{
	P_tcb pxNewTCB;
	Task_Handle_t xReturn;

	if( ( pxTaskBuffer != NULL ) && ( puxStackBuffer != NULL ) )
	{		
		pxNewTCB = ( P_tcb ) pxTaskBuffer; 
		pxNewTCB->pxStack = ( Stack_Type_p ) puxStackBuffer;

		/* 创建新的任务 */
		prv_Initialise_New_Task( pxTaskCode,        /* 任务入口 */
                              pcName,            /* 任务名称，字符串形式 */
                              ulStackDepth,      /* 任务栈大小，单位为字 */ 
                              pvParameters,      /* 任务形参 */
							  uxPriority,        /* 优先级*/
                              &xReturn,          /* 任务句柄 */ 
                              pxNewTCB);         /* 任务栈起始地址 */      
		/* 将任务添加到就绪列表 */
		prv_Add_New_Task_To_Ready_List( pxNewTCB );
	}
	else
	{
		xReturn = NULL;
	}
	/* 返回任务句柄，如果任务创建成功，此时xReturn应该指向任务控制块 */
    return xReturn;
}
#endif /* config_SUPPORT_STATIC_ALLOCATION */
/*-----------------------------创建新的任务---------------------------*/
static void prv_Initialise_New_Task( 	Task_Function_t pxTaskCode,              /* 任务入口 */
									const char * const pcName,              /* 任务名称，字符串形式 */
									const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
									void * const pvParameters,              /* 任务形参 */
								    U_Base_Type_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
									Task_Handle_t * const pxCreatedTask,     /* 任务句柄 */
									P_tcb pxNewTCB )                       /* 任务控制块指针 */
 
{
    Stack_Type_p pxTopOfStack;
	U_Base_Type_t x;	
	
	/* 获取栈顶地址 */
	pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
	//pxTopOfStack = ( StackType_t * ) ( ( ( portPOINTER_SIZE_TYPE ) pxTopOfStack ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );
	/* 向下做8字节对齐 */
	pxTopOfStack = ( Stack_Type_p ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );	

	/* 将任务的名字存储在TCB中 */
	for( x = ( U_Base_Type_t ) 0; x < ( U_Base_Type_t ) config_MAX_TASK_NAME_LEN; x++ )
	{
		pxNewTCB->pcTaskName[ x ] = pcName[ x ];

		if( pcName[ x ] == 0x00 )
		{
			break;
		}
	}
	/* 任务名字的长度不能超过configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ config_MAX_TASK_NAME_LEN - 1 ] = '\0';

    /* 初始化TCB中的xStateListItem节点 */
    v_List_Initialise_Item( &( pxNewTCB->xStateListItem ) );
    /* 设置xStateListItem节点的拥有者 */
	list_SET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
    
	/* 初始化优先级 */
	if( uxPriority >= ( U_Base_Type_t ) config_MAX_PRIORITIES )
	{
		uxPriority = ( U_Base_Type_t ) config_MAX_PRIORITIES - ( U_Base_Type_t ) 1U;
	}
	pxNewTCB->uxPriority = uxPriority;
	 
    /* 初始化任务栈 */
	pxNewTCB->pxTopOfStack = px_Port_Initialise_Stack( pxTopOfStack, pxTaskCode, pvParameters );   

	/* 让任务句柄指向任务控制块 */
    if( ( void * ) pxCreatedTask != NULL )
	{		
		*pxCreatedTask = ( Task_Handle_t ) pxNewTCB;
	}
} 

/* ------------------------初始化任务相关的列表------------------------ */
void prv_Initialise_Task_Lists( void )
{
    U_Base_Type_t uxPriority;
    
    
    for( uxPriority = ( U_Base_Type_t ) 0U; uxPriority < ( U_Base_Type_t ) config_MAX_PRIORITIES; uxPriority++ )
	{
		v_List_Initialise( &( px_Ready_Tasks_Lists[ uxPriority ] ) );
	}
	
	//任务延时
	v_List_Initialise( &x_Delayed_Task_List1 );
	v_List_Initialise( &x_Delayed_Task_List2 );
    
    px_Delayed_Task_List = &x_Delayed_Task_List1;
	px_Overflow_Delayed_Task_List = &x_Delayed_Task_List2;
	
}

static void prv_Add_New_Task_To_Ready_List( P_tcb pxNewTCB )
{
	/* 进入临界段 */
	task_ENTER_CRITICAL();
	{
		/* 全局任务计时器加一操作 */
        uxCurrentNumberOfTasks++;
        
        /* 如果pxCurrentTCB为空，则将pxCurrentTCB指向新创建的任务 */
		if( pxCurrentTCB == NULL )
		{
			pxCurrentTCB = pxNewTCB;

			/* 如果是第一次创建任务，则需要初始化任务相关的列表 */
            if( uxCurrentNumberOfTasks == ( U_Base_Type_t ) 1 )
			{
				/* 初始化任务相关的列表 */
                prv_Initialise_Task_Lists();
			}
		}
		else /* 如果pxCurrentTCB不为空，则根据任务的优先级将pxCurrentTCB指向最高优先级任务的TCB */
		{
				if( pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority )
				{
					pxCurrentTCB = pxNewTCB;
				}
		}
		uxTaskNumber++;
        
        /* 将任务添加到就绪列表 */
        prv_Add_Task_To_Ready_List( pxNewTCB );

	}
	/* 退出临界段 */
	task_EXIT_CRITICAL();
}



extern T_tcb Task1TCB;
extern T_tcb Task2TCB;
extern T_tcb IdleTaskTCB;
void v_Application_Get_Idle_Task_Memory( T_tcb **ppxIdleTaskTCBBuffer, 
                                    Stack_Type_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize );
void v_Task_Start_Scheduler( void )
{
	/*======================================创建空闲任务start==============================================*/     
    P_tcb pxIdleTaskTCBBuffer = NULL;               /* 用于指向空闲任务控制块 */
    Stack_Type_t *pxIdleTaskStackBuffer = NULL;       /* 用于空闲任务栈起始地址 */
    uint32_t ulIdleTaskStackSize;
    
    /* 获取空闲任务的内存：任务栈和任务TCB */
    v_Application_Get_Idle_Task_Memory( &pxIdleTaskTCBBuffer, 
                                   &pxIdleTaskStackBuffer, 
                                   &ulIdleTaskStackSize );    
    
    xIdleTaskHandle = x_Task_Create_Static( (Task_Function_t)prvIdleTask,              /* 任务入口 */
					                     (char *)"IDLE",                           /* 任务名称，字符串形式 */
					                     (uint32_t)ulIdleTaskStackSize ,           /* 任务栈大小，单位为字 */
					                     (void *) NULL,                            /* 任务形参 */
										(U_Base_Type_t) task_IDLE_PRIORITY,             /* 任务优先级，数值越大，优先级越高 */
					                     (Stack_Type_t *)pxIdleTaskStackBuffer,     /* 任务栈起始地址 */
					                     (P_tcb)pxIdleTaskTCBBuffer );           /* 任务控制块 */
    /*======================================创建空闲任务end================================================*/
      
    /* 手动指定第一个运行的任务 */
   // pxCurrentTCB = &Task1TCB;
															 
		/* 延时初始化*/													 
    xNextTaskUnblockTime = port_MAX_DELAY;
    xTickCount = ( Tick_Type_t ) 0U;

		/* 初始化系统时基计数器 */
    //xTickCount = ( Tick_Type_t ) 0U;
															 
    /* 启动调度器 */
    if( x_Port_Start_Scheduler() != pd_FALSE )
    {
        /* 调度器启动成功，则不会返回，即不会来到这里 */
    }
}
static port_TASK_FUNCTION( prvIdleTask, pvParameters )
{
	/* 防止编译器的警告 */
	( void ) pvParameters;
    
    for(;;)
    {
        /* 空闲任务暂时什么都不做 */
    }
}

void v_Task_Switch_Context( void )
{    
	/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */
    task_SELECT_HIGHEST_PRIORITY_TASK();
}
void v_Task_Delay( const Tick_Type_t xTicksToDelay )
{
	/* 将任务插入到延时列表 */
    prv_Add_Current_Task_To_Delayed_List( xTicksToDelay );

    /* 任务切换 */
    task_YIELD();
}

Base_Type_t x_Task_Increment_Tick( void )
{
    P_tcb pxTCB = NULL;
//    BaseType_t i = 0;
	Tick_Type_t xItemValue;
     Base_Type_t xSwitchRequired = pd_FALSE;
    /* 更新系统时基计数器xTickCount，xTickCount是一个在port.c中定义的全局变量 */
    const Tick_Type_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;

	/* 如果xConstTickCount溢出，则切换延时列表 */
	if( xConstTickCount == ( Tick_Type_t ) 0U )
	{
		task_SWITCH_DELAYED_LISTS();
	}

	/* 最近的延时任务延时到期 */
	if( xConstTickCount >= xNextTaskUnblockTime )
	{
		for( ;; )
		{
			if( list_LIST_IS_EMPTY( px_Delayed_Task_List ) != pd_FALSE )
			{
				/* 延时列表为空，设置xNextTaskUnblockTime为可能的最大值 */
				xNextTaskUnblockTime = port_MAX_DELAY;
				break;
			}
			else /* 延时列表不为空 */
			{
				pxTCB = ( P_tcb ) list_GET_OWNER_OF_HEAD_ENTRY( px_Delayed_Task_List );
				xItemValue = list_GET_LIST_ITEM_VALUE( &( pxTCB->xStateListItem ) );

				/* 直到将延时列表中所有延时到期的任务移除才跳出for循环 */
                if( xConstTickCount < xItemValue )
				{
					xNextTaskUnblockTime = xItemValue;
					break;
				}

				/* 将任务从延时列表移除，消除等待状态 */
				( void ) ux_List_Remove( &( pxTCB->xStateListItem ) );

				/* 将解除等待的任务添加到就绪列表 */
				prv_Add_Task_To_Ready_List( pxTCB );
				
								#if (  config_USE_PREEMPTION == 1 )
                {
                    if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
                    {
                        xSwitchRequired = pd_TRUE;
                    }
                }
                #endif /* config_USE_PREEMPTION */
			}
		}
	}/* xConstTickCount >= xNextTaskUnblockTime */
	#if ( ( config_USE_PREEMPTION == 1 ) && ( config_USE_TIME_SLICING == 1 ) )
    {
        if( list_CURRENT_LIST_LENGTH( &( px_Ready_Tasks_Lists[ pxCurrentTCB->uxPriority ] ) ) 
                                     > ( UBaseType_t ) 1 )
        {
            xSwitchRequired = pd_TRUE;
        }
    }
    #endif /* ( ( config_USE_PREEMPTION == 1 ) && ( config_USE_TIME_SLICING == 1 ) ) */

	return xSwitchRequired;
}


static void prv_Add_Current_Task_To_Delayed_List( Tick_Type_t xTicksToWait )
{
    Tick_Type_t xTimeToWake;
    
    /* 获取系统时基计数器xTickCount的值 */
    const Tick_Type_t xConstTickCount = xTickCount;

    /* 将任务从就绪列表中移除 */
	if( ux_List_Remove( &( pxCurrentTCB->xStateListItem ) ) == ( U_Base_Type_t ) 0 )
	{
		/* 将任务在优先级位图中对应的位清除 */
        port_RESET_READY_PRIORITY( pxCurrentTCB->uxPriority, uxTopReadyPriority );
	}

    /* 计算延时到期时，系统时基计数器xTickCount的值是多少 */
    xTimeToWake = xConstTickCount + xTicksToWait;

    /* 将延时到期的值设置为节点的排序值 */
    list_SET_LIST_ITEM_VALUE( &( pxCurrentTCB->xStateListItem ), xTimeToWake );

    /* 溢出 */
    if( xTimeToWake < xConstTickCount )
    {
        v_List_Insert( px_Overflow_Delayed_Task_List, &( pxCurrentTCB->xStateListItem ) );
    }
    else /* 没有溢出 */
    {

        v_List_Insert( px_Delayed_Task_List, &( pxCurrentTCB->xStateListItem ) );

        /* 更新下一个任务解锁时刻变量xNextTaskUnblockTime的值 */
        if( xTimeToWake < xNextTaskUnblockTime )
        {
            xNextTaskUnblockTime = xTimeToWake;
        }
    }	
}


static void prv_Reset_Next_Task_Unblock_Time( void )
{
    P_tcb pxTCB;

	if( list_LIST_IS_EMPTY( px_Delayed_Task_List ) != pd_FALSE )
	{
		xNextTaskUnblockTime = port_MAX_DELAY;
	}
	else
	{
		( pxTCB ) = ( P_tcb ) list_GET_OWNER_OF_HEAD_ENTRY( px_Delayed_Task_List );
		xNextTaskUnblockTime = list_GET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
	}
}
