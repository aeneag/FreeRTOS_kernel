#ifndef __PROJDEFS_H
#define __PROJDEFS_H

typedef void (*Task_Function_t)( void * );

#define pd_FALSE		( ( Base_Type_t ) 0 )
#define pd_TRUE			( ( Base_Type_t ) 1 )

#define pd_PASS			( pd_TRUE )
#define pd_FAIL			( pd_FALSE )

#endif // !__PROJDEFS_H
