#ifndef _RABBITMQ_COMMON

#ifndef WIN32
enum BOOL{
	FALSE = 0,
	TRUE = 1,
};
typedef enum BOOL BOOL;
typedef unsigned int uint32_t;
#else
#include <windows.h>
#endif

#define _RABBITMQ_COMMON
#endif
