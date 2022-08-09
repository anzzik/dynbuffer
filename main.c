#include <stdio.h>
#include <string.h>

#include "buffer.h"

int main(void)
{
	Buffer_t *b = buffer_new();

	buffer_append(b, "AAAA", 4);
	buffer_append(b, "BBBB", 4);
	buffer_append(b, "CCCC", 4);
	buffer_append(b, "DDDD", 4);
	buffer_append(b, "EEEE", 4);
	buffer_append(b, "FFFF", 4);

	buffer_load_from_file(b, "testfile.txt");
	buffer_write_to_file(b, "testfile_wr.txt");

	/* char debug[255] = {'\0'}; */


	printf("Original buffer:\n");
	buffer_print(b);
	return 0;

	char *replace = "cccc";
	buffer_replace_data_at_offset(b, 8, replace, 4);
	printf("\nAfter replace:\n");
	buffer_print(b);

	return 0;
	/* buffer_insert_at_offset(b, 23, "GGGG", 4); */
	/* printf("\nAfter data inserting:\n"); */
	/* buffer_print(b); */

	buffer_delete_data_at_offset(b, 8, 4);
	printf("\nAfter data deleting:\n");
	buffer_print(b);
	/* buffer_delete_data_at_offset(b, 0, 3); */
	/* buffer_print(b); */

	/* buffer_foreach_chunk(b, 0, (void (*)(Chunk_t*, void*))chunk_print); */

	buffer_free(b);
	return 0;
}

