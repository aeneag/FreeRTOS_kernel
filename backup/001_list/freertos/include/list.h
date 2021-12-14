/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * list.h       : 任务链表头文件
 *----------------------------------------------------------------*/
#ifndef __LIST__H
#define __LIST__H
#include "FreeRTOS.h"
#include <stdlib.h>
/*
*************************************************************************
*                               链表结构体 
*************************************************************************
*/

/*----------------------------链表节点-----------------------------*/
typedef struct x_list_item{      		/* 链表节点结构体定义 */
  	Tick_Type_t xItemValue;      		/* 辅助值，用于帮助节点做顺序排列 */			
	struct x_list_item *  pxNext;       /* 指向链表下一个节点 */		
	struct x_list_item *  pxPrevious;   /* 指向链表前一个节点 */	
	void * pvOwner;				 		/* 指向拥有该节点的内核对象，通常是TCB */
	void * pvContainer;		     		/* 指向该节点所在的链表 */
}T_list_item,*P_list_item;
/*---------------任务根节点，双向链表结尾，根节点中的变量---------------*/
typedef struct x_min_list_item{
	Tick_Type_t xItemValue;              /* 辅助值，用于帮助节点做升序排列 */
	struct x_list_item  *pxNext;         /* 指向链表下一个节点 */
	struct x_list_item  *pxPrevious;     /* 指向链表前一个节点 */
}T_min_list_item,*P_min_list_item;
/*----------------------------任务根节点-----------------------------*/
typedef struct x_list
{
	U_Base_Type_t 	uxNumberOfItems;    /* 链表节点计数器 */
	P_list_item   	pxIndex;			/* 链表节点索引指针 */
	T_min_list_item xListEnd;			/* 链表最后一个节点 */
} T_list,*P_list;
/*
*************************************************************************
*                               函数声明 
*************************************************************************
*/
void v_List_Initialise_Item( P_list_item const pxItem );/* 链表节点初始化 */
void v_List_Initialise( T_list * const pxList ); /* 链表根节点初始化 */
void v_List_Insert_End( P_list const pxList,  P_list_item  const pxNewListItem );/* 将节点插入到链表的尾部 */
void v_List_Insert( P_list const pxList, P_list_item  const pxNewListItem );/* 将节点按照升序排列插入到链表 */
U_Base_Type_t uxListRemove( P_list_item const pxItemToRemove );/* 将节点从链表中删除 */
/*
*************************************************************************
*                               链表带参宏
*************************************************************************
*/

/* 初始化节点的拥有者 */
#define list_SET_LIST_ITEM_OWNER( pxListItem, pxOwner )		( ( pxListItem )->pvOwner = ( void * ) ( pxOwner ) )
/* 获取节点拥有者 */
#define list_GET_LIST_ITEM_OWNER( pxListItem )				( ( pxListItem )->pvOwner )
/* 初始化节点排序辅助值 */
#define list_SET_LIST_ITEM_VALUE( pxListItem, xValue )		( ( pxListItem )->xItemValue = ( xValue ) )
/* 获取节点排序辅助值 */
#define list_GET_LIST_ITEM_VALUE( pxListItem )				( ( pxListItem )->xItemValue )
/* 获取链表根节点的节点计数器的值 */
#define list_GET_ITEM_VALUE_OF_HEAD_ENTRY( pxList )			( ( ( pxList )->xListEnd ).pxNext->xItemValue )
/* 获取链表的入口节点 */
#define list_GET_HEAD_ENTRY( pxList )						( ( ( pxList )->xListEnd ).pxNext )
/* 获取链表的第一个节点 */
#define list_GET_NEXT( pxListItem )							( ( pxListItem )->pxNext )
/* 获取链表的最后一个节点 */
#define list_GET_END_MARKER( pxList )						( ( P_list_item const ) ( &( ( pxList )->xListEnd ) ) )
/* 判断链表是否为空 */
#define list_LIST_IS_EMPTY( pxList )						( ( Base_Type_t ) ( ( pxList )->uxNumberOfItems == ( U_Base_Type_t ) 0 ) )
/* 获取链表的节点数 */
#define list_CURRENT_LIST_LENGTH( pxList )					( ( pxList )->uxNumberOfItems )

/* 获取链表节点的OWNER，即TCB */
#define list_GET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )										\
{																							\
	P_list const pxConstList = ( pxList );											        \
	/* 节点索引指向链表第一个节点调整节点索引指针，指向下一个节点，
    如果当前链表有N个节点，当第N次调用该函数时，pxInedex则指向第N个节点 */					     \
	( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;							\
	/* 当前链表为空 */                                                                       \
	if( ( void * ) ( pxConstList )->pxIndex == ( void * ) &( ( pxConstList )->xListEnd ) )	\
	{																						\
		( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;						\
	}																						\
	/* 获取节点的OWNER，即TCB */                                                             \
	( pxTCB ) = ( pxConstList )->pxIndex->pvOwner;											\
}

#define list_GET_OWNER_OF_HEAD_ENTRY( pxList )  ( (&( ( pxList )->xListEnd ))->pxNext->pvOwner )





#endif // !__LIST__H
