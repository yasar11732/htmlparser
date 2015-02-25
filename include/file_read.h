#ifndef _FILE_READ_H
#define _FILE_READ_H

#define BUFFER_SIZE 1024
#include <stddef.h>

void *realloc(void *, size_t);
void *memcpy(void *, const void *, size_t);

struct file_data {
	char *data;
	size_t numChars;
};

struct file_data read_file(const char *);

#endif