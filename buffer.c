#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

Chunk_t *chunk_new(char* c_data, size_t c_size)
{
	Chunk_t *c = calloc(1, sizeof(Chunk_t));

	c->index = 0;
	c->size  = c_size;
	c->data  = calloc(1, CHUNK_SZ + 1);
	c->next  = 0;

	memcpy(c->data, c_data, c_size);

	return c;
}

void chunk_free(Chunk_t *c)
{
	printf("Chunk #%d freed (%lu bytes).\n", c->index, c->size);
	free(c->data);
	free(c);
}

void chunk_print(Chunk_t *c)
{
	c->data[c->size] = '\0';
	/* printf("index: %d\n", c->index); */
	/* printf("s_offset: %d\n", c->s_offset); */
	/* printf("e_offset: %d\n", c->e_offset); */
	/* printf("size: %d\n", c->size); */
	/* printf("data: %s\n", c->data); */
	/* printf("------------\n\n"); */

	printf("%s", c->data);
}

void buffer_append_char(Buffer_t *b, char ch)
{
	Chunk_t *c = b->c_head;
	while (c->next)
		c = c->next;

	if (c->size < CHUNK_SZ)
	{
		c->data[c->size] = ch;
		c->data[c->size + 1] = '\0';

		c->size++;
		c->e_offset++;
		b->b_size++;
	}
	else
	{
		char s[2] = {'\0'};
		sprintf(s, "%c", ch);
		buffer_append(b, s, 1);
	}
}

Buffer_t *buffer_new()
{
	Buffer_t *b = calloc(1, sizeof(Buffer_t));

	b->c_head = 0;
	b->c_def_size = CHUNK_SZ;
	b->c_count = 0;
	b->b_size = 0;

	return b;
}

Buffer_t *buffer_create_empty()
{
	Buffer_t *b = buffer_new();
	return b;
}

Buffer_t *buffer_create_from_string(char *str)
{
	Buffer_t *b = buffer_create_empty();
	buffer_insert_at_offset(b, 0, str, strlen(str));

	return b;
}

void buffer_free(Buffer_t *b)
{
	printf("Freeing %lu byte buffer.\n", b->b_size);
	buffer_foreach_chunk(b, 0, (void (*)(Chunk_t*, void*))chunk_free);
	free(b);
}

int buffer_create_chunk(Buffer_t* b, int index, char *data, size_t size)
{
	if (index > b->c_count)
		return -1;

	Chunk_t *c   = chunk_new(data, size);
	Chunk_t **it = &b->c_head;

	for (int i = 0; i < index && i < b->c_count; i++)
		it = &(*it)->next;

	if (index < b->c_count)
		c->next = *it;

	*it = c;
	b->c_count++;
	buffer_recalc_indexes(b);

	return 0;
}

int buffer_insert_at_offset(Buffer_t *b, size_t offset, char *data, size_t size)
{
	Chunk_t *c = buffer_get_chunk_at_offset(b, offset);
	if (c == 0)
	{
		if (offset == b->b_size)
		{
			buffer_append(b, data, size);
			return 0;
		}

		fprintf(stderr, "No chunk found at offset %lu\n", offset);
		return -1;
	}

	if (offset > c->s_offset)
		buffer_split_chunk(b, c, offset);

	buffer_insert_at(b, c->index, data, size);

	return 0;
}

int buffer_load_from_file(Buffer_t *b, const char* path)
{
	FILE  *fp;
	size_t f_size;
	size_t bytes_read;
	char  *data_buf;

	fp = fopen(path, "r");
	if (!fp)
	{
		fprintf(stderr, "Cannot open file: %s\n", path);
		return -1;
	}

	buffer_clear(b);

	fseek(fp, 0, SEEK_END);
	f_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data_buf = calloc(1, f_size + 1);

	bytes_read = fread(data_buf, 1, f_size, fp);
	if (bytes_read != f_size)
	{
		fprintf(stderr, "Could not read the whole file\n");
		free(data_buf);
		fclose(fp);
		return -1;
	}

	buffer_insert_at_offset(b, 0, data_buf, f_size);

	free(data_buf);
	fclose(fp);

	return 0;
}

int buffer_load_from_string(Buffer_t *b, char* str)
{
	buffer_clear(b);
	return buffer_insert_at_offset(b, 0, str, strlen(str));
}

int buffer_insert_at(Buffer_t *b, int index, char *data, size_t size)
{
	int chunks_needed = size / CHUNK_SZ;

	if (size % CHUNK_SZ > 0)
		chunks_needed++;

	char debug[CHUNK_SZ + 1] = {'\0'};

	for (int i = chunks_needed - 1; i >= 0; i--)
	{
		int offset = i * CHUNK_SZ;
		int len = size - offset;
		if (len > CHUNK_SZ)
			len = CHUNK_SZ;

		memset(debug, '\0', CHUNK_SZ + 1);
		memcpy(debug, (data + offset), len);

		buffer_create_chunk(b, index, (data + offset), len);
	}

	return 0;
}

int buffer_append(Buffer_t* b, char *data, size_t size)
{
	return buffer_insert_at(b, b->c_count, data, size);
}

int buffer_prepend(Buffer_t* b, char *data, size_t size)
{
	return buffer_insert_at(b, 0, data, size);
}

void buffer_recalc_indexes(Buffer_t *b)
{
	Chunk_t **it = &b->c_head;
	size_t b_offset = 0;

	b->c_count = 0;
	while (*it)
	{
		(*it)->index    = b->c_count;
		(*it)->s_offset = b_offset;
		(*it)->e_offset = (*it)->s_offset + (*it)->size;
		b_offset        = (*it)->e_offset;
		b->b_size       = (*it)->e_offset;

		if ((*it)->size == 0)
		{
			Chunk_t *f = *it;
			printf("0 sized chunk at index %d.\n", f->index);

			*it = (*it)->next;
			chunk_free(f);
			continue;
		}

		b->c_count++;
		it = &(*it)->next;
	}
}

Chunk_t *buffer_get_chunk_at_index(Buffer_t *b, int index)
{
	Chunk_t *c = b->c_head;

	while (c)
	{
		if (c->index == index)
			return c;

		c = c->next;
	}

	return 0;
}

Chunk_t *buffer_get_chunk_at_offset(Buffer_t *b, size_t offset)
{
	Chunk_t *c = b->c_head;

	while (c)
	{
		if (c->s_offset <= offset && c->e_offset > offset)
			return c;

		c = c->next;
	}

	return 0;
}

int buffer_get_index_at_offset(Buffer_t *b, size_t offset)
{
	Chunk_t *c = b->c_head;

	while (c)
	{
		if (offset >= c->s_offset && offset < c->e_offset)
			return c->index;

		c = c->next;
	}

	return 0;
}

int buffer_get_data_at_offset(Buffer_t *b, size_t offset, size_t size, char* ob)
{
	Chunk_t *c_first   = buffer_get_chunk_at_offset(b, offset);
	Chunk_t *c_last    = buffer_get_chunk_at_offset(b, offset + size - 1);
	Chunk_t *c_current = c_first;
	size_t   ob_offset = 0;

	if (!c_first || !c_last)
	{
		fprintf(stderr, "offset or size oob\n");
		return -1;
	}

	while (c_current)
	{
		size_t c_offset = offset - c_current->s_offset;
		size_t chunk_l = c_current->size - c_offset;
		if (chunk_l > size)
			chunk_l = size;

		memcpy(ob + ob_offset, c_current->data + c_offset, chunk_l);

		offset     = c_current->s_offset + chunk_l;
		ob_offset += chunk_l;
		size      -= chunk_l;

		if (size == 0)
			break;

		c_current = c_current->next;
	}

	return 0;
}

int buffer_replace_data_at_offset(Buffer_t *b, size_t offset, char* data, size_t size)
{
	Chunk_t *c_first   = buffer_get_chunk_at_offset(b, offset);
	Chunk_t *c_last    = buffer_get_chunk_at_offset(b, offset + size - 1);
	Chunk_t *c_current = c_first;
	size_t   ob_offset = 0;

	if (!c_first || !c_last)
	{
		fprintf(stderr, "offset or size oob\n");
		return -1;
	}

	while (c_current)
	{
		size_t c_offset = offset - c_current->s_offset;
		size_t chunk_l = c_current->size - c_offset;
		if (chunk_l > size)
			chunk_l = size;

		memcpy(c_current->data + c_offset, data + ob_offset, chunk_l);

		offset     = c_current->s_offset + chunk_l;
		ob_offset += chunk_l;
		size      -= chunk_l;

		if (size == 0)
			break;

		c_current = c_current->next;
	}

	return 0;
}

int  buffer_write_to_file(Buffer_t *b, const char* path)
{
	FILE  *fp;
	size_t bytes_written;
	char  *data_buf;

	fp = fopen(path, "w");
	if (!fp)
	{
		fprintf(stderr, "Cannot open file: %s\n", path);
		return -1;
	}

	fseek(fp, 0, SEEK_SET);

	data_buf = calloc(1, b->b_size + 1);

	Chunk_t *c = b->c_head;
	size_t offset = 0;
	while (c)
	{
		memcpy(data_buf + offset, c->data, c->size);
		offset += c->size;
		c = c->next;
	}

	bytes_written = fwrite(data_buf, 1, b->b_size, fp);
	if (bytes_written != b->b_size)
	{
		fprintf(stderr, "Could not write the whole buffer\n");
		free(data_buf);
		fclose(fp);
		return -1;
	}

	free(data_buf);
	fclose(fp);

	return 0;
}

char *buffer_write_to_string(Buffer_t *b)
{
	Chunk_t *c = b->c_head;
	size_t offset = 0;
	char *ob_str = calloc(sizeof(b->b_size) + 1, sizeof(char));

	while (c)
	{
		memcpy(ob_str + offset, c->data, c->size);
		offset += c->size;
		c = c->next;
	}

	return ob_str;
}

void buffer_clear(Buffer_t *b)
{
	buffer_foreach_chunk(b, 0, (void (*)(Chunk_t*, void*))chunk_free);
	b->c_head = 0;
	b->c_def_size = CHUNK_SZ;
	b->c_count = 0;
	b->b_size = 0;
}

int buffer_delete_data_at_offset(Buffer_t *b, size_t offset, size_t size)
{
	Chunk_t *c_first   = buffer_get_chunk_at_offset(b, offset);
	Chunk_t *c_last    = buffer_get_chunk_at_offset(b, offset + size - 1);
	Chunk_t *c_current = c_first;

	if (!c_first || !c_last)
	{
		fprintf(stderr, "offset or size oob\n");
		return -1;
	}
  
	while (c_current)
	{
		size_t c_offset = offset - c_current->s_offset;
		size_t chunk_l = c_current->size - c_offset;
		if (chunk_l > size)
			chunk_l = size;

		memset(c_current->data + c_offset, '\0', chunk_l);
		if (c_offset == 0)
		{
			if (chunk_l == c_current->size)
			{
				c_current->size = 0;
			}
			else
			{
				c_current->size = c_current->size - chunk_l;
				memmove(
					c_current->data,
					c_current->data + chunk_l,
					c_current->size);
			}
		}

		if (c_offset > 0)
		{
			if (c_offset + chunk_l == c_current->size)
			{
				c_current->size = c_offset;
			}
			else
			{
				c_current->size = chunk_l - c_offset;
				memmove(
					c_current->data + c_offset,
					c_current->data + c_offset + chunk_l,
					c_current->size);
			}
		}

		buffer_recalc_indexes(b);

		offset = c_current->s_offset + chunk_l;
		size -= chunk_l;

		if (size == 0)
			break;

		c_current = c_current->next;
	}

	return 0;
}

void buffer_print(Buffer_t *b)
{
	Chunk_t *c = b->c_head;

	while (c)
	{
		chunk_print(c);
		c = c->next;
	}

	printf("\nTotal buffer size: %lu bytes, %d chunks.\n", b->b_size, b->c_count);
}

int buffer_split_chunk(Buffer_t *b, Chunk_t *c, size_t offset)
{
	size_t c_offset = offset - c->s_offset;
	char f[CHUNK_SZ + 1] = {'\0'};
	char s[CHUNK_SZ + 1] = {'\0'};

	memcpy(f, c->data, c_offset);
	memcpy(s, c->data + c_offset, c->size - c_offset);

	buffer_insert_at(b, c->index, f, strlen(f));

	memcpy(c->data, s, strlen(s) + 1);
	c->size = strlen(s);

	buffer_recalc_indexes(b);

	return 0;
}

void buffer_foreach_chunk(Buffer_t *b, void* data, void (cb_fn(Chunk_t*, void*)))
{
	Chunk_t *c = b->c_head;

	while (c)
	{
		cb_fn(c, data);
		c = c->next;
	}
}
