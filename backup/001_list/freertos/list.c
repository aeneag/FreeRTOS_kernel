/*----------------------------------------------------------------
 * author       : Aen
 * blog			: https://aeneag.xyz
 * date         : 2021/12/13
 * list.h       : 任务链表相关实现函数
 *----------------------------------------------------------------*/
#include "list.h"
/*-----------------------链表节点初始化-----------------------------*/
void v_List_Initialise_Item( P_list_item const pxItem )
{
	/* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
	pxItem->pvContainer = NULL;
}
/*-----------------------链表根节点初始化-----------------------------*/
void v_List_Initialise( T_list * const pxList )
{
	/* 将链表索引指针指向最后一个节点 */
	pxList->pxIndex = ( P_list_item ) &( pxList->xListEnd );
	/* 将链表最后一个节点的辅助排序的值设置为最大，确保该节点就是链表的最后节点 */
	pxList->xListEnd.xItemValue = port_MAX_DELAY;
    /* 将最后一个节点的pxNext和pxPrevious指针均指向节点自身，表示链表为空 */
	pxList->xListEnd.pxNext = ( P_list_item ) &( pxList->xListEnd );
	pxList->xListEnd.pxPrevious = ( P_list_item ) &( pxList->xListEnd );
	/* 初始化链表节点计数器的值为0，表示链表为空 */
	pxList->uxNumberOfItems = ( U_Base_Type_t ) 0U;
}
/*------------------------将节点插入到链表的尾部------------------------*/
void v_List_Insert_End( P_list const pxList,  P_list_item  const pxNewListItem )
{
	P_list_item const pxIndex = pxList->pxIndex;
    /* 双向链表插入操作， */
	pxNewListItem->pxNext = pxIndex;
	pxNewListItem->pxPrevious = pxIndex->pxPrevious;
	pxIndex->pxPrevious->pxNext = pxNewListItem;
	pxIndex->pxPrevious = pxNewListItem;
	/* 记住该节点所在的链表 */
	pxNewListItem->pvContainer = ( void * ) pxList;
	/* 链表节点计数器++ */
	++( pxList->uxNumberOfItems );
}
/*------------------------将节点按照升序排列插入到链表------------------------*/
void v_List_Insert( P_list const pxList, P_list_item  const pxNewListItem )
{
	P_list_item pxIterator;
	/* 获取节点的排序辅助值 */
	const Tick_Type_t xValueOfInsertion = pxNewListItem->xItemValue;
	/* 寻找节点要插入的位置 */
	if( xValueOfInsertion == port_MAX_DELAY )
	{
		pxIterator = pxList->xListEnd.pxPrevious;
	}
	else
	{
		for( pxIterator = ( P_list_item ) &( pxList->xListEnd );
		     pxIterator->pxNext->xItemValue <= xValueOfInsertion; 
			 pxIterator = pxIterator->pxNext );
	}
    /* 插入操作 */
	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxNext->pxPrevious = pxNewListItem;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext = pxNewListItem;
	/* 记住该节点所在的链表 */
	pxNewListItem->pvContainer = ( void * ) pxList;
	/* 链表节点计数器++ */
	++( pxList->uxNumberOfItems );
}
/*-----------------------将节点从链表中删除-------------------------*/
U_Base_Type_t uxListRemove( P_list_item const pxItemToRemove )
{
	/* 获取节点所在的链表 */
	P_list const pxList = ( P_list ) pxItemToRemove->pvContainer;

	pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
	pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

	/* 调整链表的节点索引指针，确保索引指向的仍未有效节点 */
	if( pxList->pxIndex == pxItemToRemove )
	{
		pxList->pxIndex = pxItemToRemove->pxPrevious;
	}

	/* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
	pxItemToRemove->pvContainer = NULL;
	/* 链表节点计数器-- */
	--( pxList->uxNumberOfItems );
	/* 返回链表中剩余节点的个数 */
	return pxList->uxNumberOfItems;
}

