#include <stdio.h>
#include <string.h>

#include "buffer.h"

int main(void)
{
	Buffer_t *b = buffer_create_empty();

	buffer_load_from_string(b ,"AAABBBCCC");

	buffer_append_char(b, 'f');
	buffer_append_char(b, 'f');
	buffer_append_char(b, 'f');
	buffer_append_char(b, 'f');
	buffer_append_char(b, 'f');
	buffer_append_char(b, 'f');
	buffer_print(b);

	buffer_load_from_file(b, "testfile.txt");
	buffer_write_to_file(b, "testfile_wr.txt");

	char *replace = "cccc";
	buffer_replace_data_at_offset(b, 8, replace, 4);
	printf("\nAfter replace:\n");
	buffer_print(b);

	buffer_insert_at_offset(b, 23, "GGGG", 4);
	buffer_print(b);

	buffer_delete_data_at_offset(b, 8, 4);
	printf("\nAfter data deleting:\n");
	buffer_print(b);

	buffer_free(b);
	return 0;
}

