#ifndef __STACK_H_DEFINED_
#define __STACK_H_DEFINED_

/* new stack */
#define DEFINE_STACK(type)         \
	typedef struct tagStack_##type  \
	{                           \
		type    *pStackBuffer;  \
		int		nStackDepth;    \
		int		nStackTop;      \
	}STACK_##type

DEFINE_STACK( PVOID );   // STACK_PVOID
DEFINE_STACK( CHAR  );   // STACK_CHAR


#define STACK_IS_FULL( pStack )     \
    (((pStack)->nStackTop+1 > (pStack)->nStackDepth) ? TRUE : FALSE)

#define STACK_IS_EMPTY( pStack )    \
    (((pStack)->nStackTop <= 0) ? TRUE : FALSE)

#define STACK_INIT(pStack, pStackBuf, nStackDep)\
    (pStack)->pStackBuffer = (pStackBuf),       \
    (pStack)->nStackDepth  = (nStackDep),       \
    (pStack)->nStackTop    = 0

#define STACK_PUSH( pStack, ulItem )    \
    ((pStack)->pStackBuffer[(pStack)->nStackTop ++] = (ulItem))

#define STACK_POP( pStack ) \
    ((pStack)->pStackBuffer[--(pStack)->nStackTop])

#define STACK_SAFELY_POP(pStack, ulItem)	\
	(!STACK_IS_EMPTY(pStack) ? ((ulItem)=STACK_POP((pStack)), TRUE) : FALSE)

#define STACK_TOP( pStack ) \
    ((pStack)->pStackBuffer[(pStack)->nStackTop-1])

#define STACK_DEPTH( pStack )   \
    ((pStack)->nStackTop)

#define STACK_POP_BOTTOM( pStack )  \
    ((pStack)->pStackBuffer[(pStack)->nStackTop = 0])

#define STACK_SET_EMPTY( pStack )\
	((pStack)->nStackTop = 0)

#endif
