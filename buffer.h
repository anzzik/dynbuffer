#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#define CHUNK_SZ 4

typedef struct Chunk_s
{
	int    index;
	size_t size;
	size_t s_offset;
	size_t e_offset;
	char  *data;

	struct Chunk_s *next;
} Chunk_t;

typedef struct Buffer_s
{
	Chunk_t  *c_head;
	size_t    c_def_size;
	int       c_count;
	size_t    b_size;

} Buffer_t;

/* API */

void buffer_append_char(Buffer_t *b, char ch);
void buffer_clear(Buffer_t *b);
Buffer_t *buffer_create_empty();
Buffer_t *buffer_create_from_string(char *str);
int  buffer_delete_data_at_offset(Buffer_t *b, size_t offset, size_t size);
void buffer_free(Buffer_t *b);
int  buffer_get_data_at_offset(Buffer_t *b, size_t offset, size_t size, char* ob);
int  buffer_insert_at_offset(Buffer_t *b, size_t offset, char *data, size_t size);
int  buffer_load_from_file(Buffer_t *b, const char* path);
int  buffer_load_from_string(Buffer_t *b, char* str);
void buffer_print(Buffer_t *b);
int  buffer_replace_data_at_offset(Buffer_t *b, size_t offset, char* data, size_t size);
int  buffer_write_to_file(Buffer_t *b, const char* path);
char *buffer_write_to_string(Buffer_t *b);

/* For internal use */

Chunk_t *chunk_new(char* c_data, size_t c_size);
void     chunk_free(Chunk_t *c);
void     chunk_print(Chunk_t *c);

Buffer_t *buffer_new();
int  buffer_append(Buffer_t* b, char *data, size_t size);
int  buffer_create_chunk(Buffer_t* b, int index, char *data, size_t size);
void buffer_foreach_chunk(Buffer_t *b, void* data, void (cb_fn(Chunk_t*, void*)));
Chunk_t *buffer_get_chunk_at_index(Buffer_t *b, int index);
Chunk_t *buffer_get_chunk_at_offset(Buffer_t *b, size_t offset);
int  buffer_get_index_at_offset(Buffer_t *b, size_t offset);
int  buffer_insert_at(Buffer_t *b, int index, char *data, size_t size);
int  buffer_prepend(Buffer_t* b, char *data, size_t size);
void buffer_recalc_indexes(Buffer_t *b);
int  buffer_split_chunk(Buffer_t *b, Chunk_t *c, size_t offset);

#endif
