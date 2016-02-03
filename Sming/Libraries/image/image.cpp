//*****************************************************************************
//
// Image loader for Sming
// Public domain, Sebastien Leclerc
//*****************************************************************************
#include "../../SmingCore/SmingCore.h"
#include "../../SmingCore/FileSystem.h"

#include "image.h"


static Vector<image_mem_allocation> ptrlist;
static uint32 startStack;
register int sp asm ("sp");

// implementation of stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



// stb_image callbacks
int read(void* file, char * data, int size) {
	file_t* stream = static_cast<file_t*>(file);
	if (!fileIsEOF(*stream))
		return static_cast<int>(fileRead(*stream, (void *) data, size));
	else
		return 0;
}
void skip(void* file, int size) {
	file_t* stream = static_cast<file_t*>(file);
	fileSeek(*stream, size, eSO_CurrentPos);
}
int eof(void* file) {
	file_t* stream = static_cast<file_t*>(file);
	return fileIsEOF(*stream);
}
image_struct * image_load(String file_name, int req_channels) {
	// Setup the stb_image callbacks

	stbi_io_callbacks * callbacks = new stbi_io_callbacks;
	callbacks->read = &read;
	callbacks->skip = &skip;
	callbacks->eof = &eof;
	image_struct * pnewImage = (image_struct*)malloc(sizeof(image_struct));

	if (!pnewImage) {
		Serial.println("Could not allocate memory for image");
		return null;
	}
	memset(pnewImage, 0, sizeof(image_struct));
	IMAGE_MEMDEBUG_STORE_MEM;
	IMAGE_MEMDEBUG_PRINT_STACK(file_name);
	file_t * pFile = new file_t;
	*pFile = fileOpen(file_name, eFO_ReadOnly);

	if (*pFile < 0) {
		Serial.printf("Unable to open file. Error: %x\n",
				fileLastError(*pFile));

		Vector<String> list = fileList();
		Serial.println("File List:");
		for (int i = 0; i < list.count(); i++)
			Serial.printf("%6i	%s	\n", fileGetSize(list[i]), list[i].c_str());
		free(callbacks);
		free(pnewImage);
		return null;
	}
	pnewImage->fileName = file_name;
	Serial.print("Loading file ");	Serial.println(file_name.c_str());
	// Load the image and get a pointer to the pixels in memory
	pnewImage->imagedata = stbi_load_from_callbacks(callbacks, pFile,
			&pnewImage->width, &pnewImage->height, &pnewImage->pchannels,
			req_channels);
	if (pnewImage->imagedata == NULL) {
		Serial.printf("Could not load %s ERROR: %s\n", file_name.c_str(),
				stbi_failure_reason());
		free(pnewImage);
		pnewImage = NULL;
	}
	fileClose(*pFile);
	free(callbacks);
	free(pFile);
	return pnewImage;
}
void image_free(image_struct * imageData) {
	stbi_image_free(imageData->imagedata);
}
void image_dump_pointers() {
	// this function is useful to determine how much space
	// is occupied by loaded images at any given point of time
	uint32 totalSize=0;
	Serial.print("\n========================================\n");

	if (ptrlist.count() > 0) {
		Serial.print("Current memory pointers allocated: ");
		Serial.println(ptrlist.count());
	}
	else{
		Serial.print("No pointers allocated for images.\n");
	}
	for (int i = 0; i < ptrlist.count(); i++) {
		Serial.print((unsigned long) ptrlist[i].pointer, HEX);
		Serial.print("(");
		Serial.print(ptrlist[i].size);
		Serial.print(" bytes)\n");
		totalSize+=ptrlist[i].size;
	}
	Serial.print("\n\nTotal bytes:");
	Serial.print(totalSize);
	Serial.println("\n========================================\n");
}

void remove_pointer_from_list(void * ptr){
	for(int i=0;i<ptrlist.count();i++){
		if(ptrlist[i].pointer == ptr) {
			ptrlist.remove(i);
			break;
		}
	}
}
void * my_malloc(size_t size, const char * filename,uint32 lineno) {
	image_mem_allocation newPtrElement;
	uint32 currentmem = system_get_free_heap_size();
	void * newPtr = malloc(size);
#ifdef __IMAGE_MEMDEBUG
	Serial.printf("New pointer (%x) for %i bytes, from %s(%i), heap moved by %i bytes. Free heap: %i\n", newPtr,size,filename,lineno,currentmem-system_get_free_heap_size(),system_get_free_heap_size());
#endif
	newPtrElement.size = size;
	newPtrElement.pointer = (void *)newPtr;
	ptrlist.add((image_mem_allocation)newPtrElement);
	return newPtr;
}
void my_free(void * ptr) {
	uint32 currentmem = system_get_free_heap_size();

	remove_pointer_from_list(ptr);
	free(ptr);
#ifdef __IMAGE_MEMDEBUG
	Serial.printf("Freeing (%x) %i bytes released. Allocated pointers: %i Free heap: %i\n", ptr,system_get_free_heap_size()-currentmem,ptrlist.count(),system_get_free_heap_size());
#endif
}
void * my_realloc(void *__ptr, size_t __size) {
	image_mem_allocation newPtrElement;
	remove_pointer_from_list(__ptr);
	uint32 currentmem = system_get_free_heap_size();
	void * newPtr = realloc(__ptr,__size);
#ifdef __IMAGE_MEMDEBUG
	Serial.printf("New pointer (%x) for %i bytes, replaced (%x). heap moved by %i bytes. Free heap: %i\n", newPtr,__ptr,__size,currentmem-system_get_free_heap_size(),system_get_free_heap_size());
#endif
	newPtrElement.size = __size;
	newPtrElement.pointer = (void *)newPtr;
	ptrlist.add((image_mem_allocation)newPtrElement);
	return newPtr;
}
static void printCurStackSpace(uint32 curStack, const char * filename, uint32 lineno ) {
	Serial.printf("%s (%i) - Stack pointer (%i->%i): %i\n", filename,lineno,startStack,sp,curStack-sp);
}

