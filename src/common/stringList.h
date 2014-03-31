#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <stdlib.h>
#include <Windows.h>

#define INTSIZE_STEPSIZE 10 // size to grow _size array at each realloc to minimize resizes

typedef struct {
	TCHAR *_current;
	unsigned char *_head;
	unsigned char *_tail;
	unsigned int *_size;
	unsigned int index;
	unsigned int length;
	TCHAR *(*current)(void *This);
	TCHAR *(*pop)(void *This);
	void (*next)(void *This);
	void (*previous)(void *This);
	void (*append)(void *This, TCHAR *str);
	void (*remove)(void *This);
	void (*destroy)(void *This);
} stringList;

void initStringList(stringList *sl);

#endif // STRINGLIST_H