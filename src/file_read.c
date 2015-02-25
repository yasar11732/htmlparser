#include <stdio.h>
#include <assert.h>
#include "file_read.h"


/* Read whole file into memory */
struct file_data read_file(const char *filename) {
	FILE *f;
	char *data = NULL;
	size_t current_size = 0;
	size_t totalChars = 0;
	size_t nchars;
	struct file_data fd = {NULL, 0};
	
	if(filename) {
		f = fopen(filename, "r");
	} else {
		f = stdin;
	}
	
	if(!f) {
		perror("Unable to open file: ");
		return fd;
	}
	
	do {
		data = (char *)realloc(data, current_size + BUFFER_SIZE);
		assert(data);
		current_size+=BUFFER_SIZE;
		nchars = fread(&data[totalChars], 1, BUFFER_SIZE, f);
		if(ferror(f)) {
			perror("Couldnt read file: \n");
		}
		totalChars+=nchars;
	} while(nchars > 0);
	
	fd.data = data;
	fd.numChars = totalChars;
	return fd;
}