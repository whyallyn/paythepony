#include "stringList.h"

/* return current string from list (must free later) */
TCHAR *getStringFromList(void *This) {
	stringList *me = (stringList *)This;
	TCHAR *string = (TCHAR *)malloc(me->_size[me->index]);

	memcpy(string, me->_current, me->_size[me->index]);
	return string;
}

/* remove and return current string from list (must free later) */
TCHAR *popStringFromList(void *This) {
	stringList *me = (stringList *)This;
	TCHAR *string = me->current(me);

	me->remove(me);
	return string;
}

/* go to next string in list */
void nextStringInList(void *This) {
	stringList *me = (stringList *)This;

	// at last string in list -> go to beginning
	if (me->length == (me->index + 1)) {
		me->_current = (TCHAR *)me->_head;
		me->index = 0;
	}
	// list not empty
	else if (me->length > 0) {
		me->_current = (TCHAR *)((unsigned char *)me->_current + me->_size[me->index ]);
		me->index++;
	}
}

/* go to previous string in list */
void previousStringInList(void *This) {
	stringList *me = (stringList *)This;

	// at first string in list -> go to end
	if (me->index == 0 && me->length > 0) {
		me->_current = (TCHAR *)(me->_tail - (me->_size[me->length - 1] - sizeof(TCHAR)));
		me->index = me->length - 1;
	}
	// list not empty
	else if (me->length > 0) {
		me->_current = (TCHAR *)((unsigned char *)me->_current - me->_size[me->index - 1]);
		me->index--;
	}
}

/* append string to list */
void appendStringToList(void *This, TCHAR *str) {
	stringList *me = (stringList *)This;
	unsigned int *itmp = NULL;
	size_t oldsize = (me->_tail + sizeof(TCHAR)) - me->_head;
	size_t newsize = 0;
	size_t currentOffset = 0;
	unsigned char *ctmp = NULL;

	// increase _size array if at a step size
	if ((me->length + 1) % INTSIZE_STEPSIZE == 0) {
		itmp = (unsigned int *)realloc(me->_size, (me->length + 1 + INTSIZE_STEPSIZE) * sizeof(unsigned int));
		if (itmp)
			me->_size = itmp;
		else
			return;
	}

	// add size of new string to _size array
	me->_size[me->length] = (wcslen(str) + 1) * sizeof(TCHAR);
	
	// update size of string blob
	newsize = oldsize + me->_size[me->length];
	currentOffset = (unsigned char *)me->_current - me->_head;
	ctmp = (unsigned char *)realloc(me->_head, newsize);

	// update _head, _tail and _current and add new string
	if (ctmp) {
		me->_head = ctmp;
		me->_tail = me->_head + newsize - sizeof(TCHAR);
		me->_current = (TCHAR *)(me->_head + currentOffset);
		memcpy(me->_head+oldsize, str, me->_size[me->length]);
		me->length++;
	}
}

/* remove string from list */
void removeStringFromList(void *This) {
	stringList *me = (stringList *)This;
	size_t oldsize = (me->_tail + sizeof(TCHAR)) - me->_head;
	size_t newsize = oldsize - me->_size[me->index];
	unsigned char *ctmp = NULL;
	size_t front = (unsigned char *)me->_current - me->_head;
	size_t back = newsize - front;
	size_t inewsize = me->length - 1;
	unsigned int *itmp = NULL;
	size_t ifront = me->index * sizeof(unsigned int);
	size_t iback = (me->length - 1 - me->index) * sizeof(unsigned int);
	unsigned int newindex = 0;
	unsigned int i = 0;

	// recreate string list if removing the only string from list
	if (me->length == 1) {
		me->destroy(me);
		initStringList(me);
		return;
	}

	// increase inewsize if at a step size
	if (inewsize % INTSIZE_STEPSIZE)
		inewsize += INTSIZE_STEPSIZE;

	if (me->length > 0) {
		// create new string and int arrays with decreased sizes
		ctmp = (unsigned char *)calloc(newsize, sizeof(unsigned char *));
		memcpy(ctmp, me->_head, front);
		memcpy(ctmp + front, (unsigned char *)me->_current + me->_size[me->index], back);
		itmp = (unsigned int *)calloc(inewsize, sizeof(unsigned int));
		memcpy(itmp, me->_size, ifront);
		memcpy((unsigned char *)itmp + ifront, (unsigned char *)me->_size + ((me->index + 1) * sizeof(unsigned int)), iback);

		// store new index
		newindex = me->index % (me->length - 1);

		// cleanup old arrays
		free(me->_head);
		free(me->_size);

		// set new locations
		me->_head = ctmp;
		me->_tail = ctmp + newsize - sizeof(TCHAR);
		me->_size = itmp;
		me->index = newindex;
		me->length--;
		me->_current = (TCHAR *)me->_head;
		for (i = 0; i != me->index; i++) {
			me->_current = (TCHAR *)((unsigned char *)me->_current + me->_size[i]);
		}
	}
}

/* destroy string list */
void destroyStringList(void *This) {
	stringList me = *(stringList *)This;

	me._current = NULL;
	free(me._head);
	me._head = me._tail = NULL;
	free(me._size);
	me.index = me.length = 0;
}

/* initialize stringList */
void initStringList(stringList *sl) {
	sl->_current = (TCHAR *)calloc(1, sizeof(TCHAR));
	sl->_head = (unsigned char *)sl->_current;
	sl->_tail = (unsigned char *)sl->_current - sizeof(TCHAR);
	sl->_size = (unsigned int *)calloc(INTSIZE_STEPSIZE, sizeof(unsigned int));
	sl->_size[0] = sizeof(TCHAR);
	sl->index = 0;
	sl->length = 0;
	sl->current = &getStringFromList;
	sl->pop = &popStringFromList;
	sl->next = &nextStringInList;
	sl->previous = &previousStringInList;
	sl->append = &appendStringToList;
	sl->remove = &removeStringFromList;
	sl->destroy = &destroyStringList;
}