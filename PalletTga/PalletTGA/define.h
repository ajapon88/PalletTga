#ifndef __DEFINE_H__
#define __DEFINE_H__

#define SAFE_DELETE(ptr) do{ if(ptr){ delete ptr; ptr=NULL; } }while(false)
#define SAFE_DELETE_ARRAY(ptr) do{ if(ptr){ delete[] ptr; ptr=NULL; } }while(false)
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#define COLOR_ARGB(a, r, g, b) (((a)<<24)+((r)<<16)+((g)<<8)+(b))
#define COLOR_ALPHA(color) (((color)>>24)&0xFF)
#define COLOR_RED(color) (((color)>>16)&0xFF)
#define COLOR_GREEN(color) (((color)>>8)&0xFF)
#define COLOR_BLUE(color) ((color)&0xFF)

#ifdef _DEBUG
#define DEBUG_PAUSE() getchar()
#else
#define DEBUG_PAUSE() do{}while(false)
#endif

#endif // __DEFINE_H__