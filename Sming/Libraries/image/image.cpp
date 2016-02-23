//*****************************************************************************
//
// Image loader for Sming
// Public domain, Sebastien Leclerc
//*****************************************************************************
#include "../../SmingCore/SmingCore.h"
#include "../../SmingCore/FileSystem.h"
#include <../../SmingCore/Debug.h>

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
void  skip(void* file, int size) {
	file_t* stream = static_cast<file_t*>(file);
	fileSeek(*stream, size, eSO_CurrentPos);
}
int  eof(void* file) {
	file_t* stream = static_cast<file_t*>(file);
	return fileIsEOF(*stream);
}
image_struct *  image_load(const String file_name, int req_channels) {
	// Setup the stb_image callbacks
	image_struct * pnewImage = NULL;
	stbi_io_callbacks callbacks;
	callbacks.read = &read;
	callbacks.skip = &skip;
	callbacks.eof = &eof;
    file_t FileHandle = fileOpen(file_name, eFO_ReadOnly);

	if (FileHandle < 0) {
		debugf("Unable to open file %s. Error: %x",file_name.c_str(),fileLastError(FileHandle));
		Vector<String> list = fileList();
		debugf("File List:");
		for (int i = 0; i < list.count(); i++)
			debugf("%6i	%s", fileGetSize(list[i]), list[i].c_str());
		list.removeAllElements();
	}
	else {
		pnewImage = (image_struct*)STBI_MALLOC(sizeof(image_struct));
		if (!pnewImage) {
			debugf("Could not allocate memory for image");
		}
		else {
			memset(pnewImage, 0, sizeof(image_struct));
			pnewImage->fileName = file_name;
			//debugf("Loading file %s",file_name.c_str());
			// Load the image and get a pointer to the pixels in memory
			pnewImage->imagedata = stbi_load_from_callbacks(&callbacks, &FileHandle,
					&pnewImage->width, &pnewImage->height, &pnewImage->pchannels,
					req_channels);
			if (pnewImage->imagedata == NULL) {
				debugf("Could not load %s ERROR: %s", file_name.c_str(),
						stbi_failure_reason());
				STBI_FREE(pnewImage);
				pnewImage = NULL;
			}
		}
		fileClose(FileHandle);
	}
	return pnewImage;
}
void image_free(image_struct * imageData) {
	STBI_FREE(imageData->imagedata);
	STBI_FREE(imageData);
}
void image_dump_pointers() {
	// this function is useful to determine how much space
	// is occupied by loaded images at any given point of time
	uint32 totalSize=0;
	debugf("\n========================================");

	if (ptrlist.count() > 0) {
		debugf("Current memory pointers allocated: %u",ptrlist.count());
	}
	else{
		debugf("No pointers allocated for images.");
	}
	for (int i = 0; i < ptrlist.count(); i++) {
		debugf("%x (%u bytes)",(unsigned long) ptrlist[i].pointer,ptrlist[i].size);
		totalSize+=ptrlist[i].size;
	}
	debugf("\n\nTotal bytes: %u\n========================================", totalSize);
}

void  remove_pointer_from_list(void * ptr){
	for(int i=0;i<ptrlist.count();i++){
		if(ptrlist[i].pointer == ptr) {
			ptrlist.remove(i);
			break;
		}
	}
}
void  add_pointer_to_list(size_t size, void * newPtr){
	image_mem_allocation newPtrElement;
	newPtrElement.size = size;
	newPtrElement.pointer = (void *)newPtr;
	ptrlist.add((image_mem_allocation)newPtrElement);
}
void * my_malloc(size_t size, const char * filename,uint32 lineno) {
	void * newPtr = malloc(size);
	if(newPtr){
		memset(newPtr, 0, size);
		add_pointer_to_list(size,newPtr);
	}
	return newPtr;
}
void my_free(void * ptr) {
	if(!ptr) return;
	remove_pointer_from_list(ptr);
	free(ptr);
}
void * my_realloc(void *__ptr, size_t __size) {
	void * newPtr = NULL;
	if(__ptr)
		newPtr = realloc(__ptr,__size);
	else {
		newPtr = malloc(__size);
		if(newPtr ) {
			memset(newPtr, 0, __size);
		add_pointer_to_list(__size,newPtr);
		}
	}
	return newPtr;
}

