// int main(void){
// 	for(;;);
// }

#include "list.h"
/* 定义链表根节点 */
T_list       List_Test;
/* 定义节点 */
T_list_item  List_Item1;
T_list_item  List_Item2;
T_list_item  List_Item3;
int main(void)
{	
	
    /* 链表根节点初始化 */
    v_List_Initialise( &List_Test );
    
    /* 节点1初始化 */
    v_List_Initialise_Item( &List_Item1 );
    List_Item1.xItemValue = 1;
    
    /* 节点2初始化 */    
    v_List_Initialise_Item( &List_Item2 );
    List_Item2.xItemValue = 2;
    
    /* 节点3初始化 */
    v_List_Initialise_Item( &List_Item3 );
    List_Item3.xItemValue = 3;
    
    /* 将节点插入链表，按照升序排列 */
    v_List_Insert( &List_Test, &List_Item2 );    
    v_List_Insert( &List_Test, &List_Item1 );
    v_List_Insert( &List_Test, &List_Item3 );    
    
    for(;;)
	{
		/* 啥事不干 */
	}
}
