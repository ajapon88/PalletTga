#ifndef __DEFINE_H__
#define __DEFINE_H__

#define SAFE_DELETE(ptr) do{ if(ptr){ delete ptr; ptr=NULL; } }while(false)
#define SAFE_DELETE_ARRAY(ptr) do{ if(ptr){ delete[] ptr; ptr=NULL; } }while(false)
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))


#ifdef _DEBUG
#define DEBUG_PAUSE() getchar()
#else
#define DEBUG_PAUSE() do{}while(false)
#endif

#endif // __DEFINE_H__