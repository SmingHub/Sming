//*****************************************************************************
// image.h
//
// Image loader for Sming
// Public domain, Sebastien Leclerc
//*****************************************************************************

#ifndef IMAGE_H
#define IMAGE_H

//*****************************************************************************
//
// Select ONLY the image formats that are needed here
// as code size will grow for each decoder and this
// could lead to stack overflows/crashes

#define STBI_ONLY_JPEG
//#define STBI_ONLY_PNG
//#define STBI_ONLY_BMP
//#define STBI_ONLY_PSD
//#define STBI_ONLY_TGA
//#define STBI_ONLY_GIF
//#define STBI_ONLY_HDR
//#define STBI_ONLY_PIC
//#define STBI_ONLY_PNM

// Uncomment line below if you want access to zlib library
// while png are not activated
//#define STBI_SUPPORT_ZLIB
//
//*****************************************************************************

#define STBI_NO_SIMD
#define STBI_NO_HDR
#define STBI_ASSERT(x)

// uncomment this to get verbose messages from allocation/free
//#define __IMAGE_MEMDEBUG


#ifdef __IMAGE_MEMDEBUG
#define IMAGE_MEMDEBUG_STORE_MEM startStack = sp;
#define IMAGE_MEMDEBUG_PRINT_STACK(a) {printCurStackSpace((uint32)&a,__FILE__,__LINE__);}
#else
#define IMAGE_MEMDEBUG_PRINT_STACK(a)
#define IMAGE_MEMDEBUG_STORE_MEM
#endif

#define STBI_MALLOC(sz) my_malloc(sz,__FILE__,__LINE__)
#define STBI_REALLOC(pt,sz) my_realloc(pt,sz)
#define STBI_FREE(ptr) my_free(ptr)


#define STBI_DECODERS_COUNT (defined(STBI_ONLY_JPEG)+defined(STBI_ONLY_PNG)+defined(STBI_ONLY_BMP)+defined(STBI_ONLY_TGA)+\
		defined(STBI_ONLY_GIF)+defined(STBI_ONLY_PSD)+defined(STBI_ONLY_HDR)+defined(STBI_ONLY_PIC)+defined(STBI_ONLY_PNM)+defined(STBI_ONLY_ZLIB))
#if STBI_DECODERS_COUNT == 0
// define at least one exclusive decoder to avoid code being too large
#pragma message "image library compiled with JPEG support only."
#define STBI_ONLY_JPEG
#endif


// Include stb image library as header.  Implementation include is
// done inside image.cpp.  It needs to be included here, after
// configuration defines have been set and before declaring our own
// structures
#include "stb_image.h"

typedef struct {
	void * pointer;
	uint32 size;
} image_mem_allocation;


typedef struct {
	stbi_uc * imagedata;
	int width;
	int height;
	int pchannels;
	String fileName;
} image_struct;

image_struct * image_load(String file_name, int req_channels);
void image_free(image_struct * imageData);
void image_dump_pointers();
void * my_malloc(size_t size, const char * filename,uint32 lineno);
void my_free(void * ptr);
void * my_realloc(void *__ptr, size_t __size);
static void printCurStackSpace(uint32 curStack, const char * filename, uint32 lineno );




#endif // IMAGE_H
