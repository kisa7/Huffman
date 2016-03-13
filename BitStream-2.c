#define _CRTDBG_MAP_ALLOC
//#define TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include <limits.h>
#include <assert.h>

#include "BitStream-2.h"

#define MY_EOF -1

struct bitstream_t
{
	unsigned* buf;
	size_t capacity;
	size_t length;
	size_t pos;
};

bitstream_t* BitStreamCreateFromFile(FILE *f)
{
	bitstream_t* bs = malloc(sizeof(bitstream_t));
	unsigned int size = 0;
	fread(&size, sizeof(size), 1, f);
	bs->capacity = size / sizeof(unsigned);
	fread(&bs->length, sizeof(bs->length), 1, f);
	bs->buf = malloc(size);
	fread(bs->buf, size, 1, f);
	bs->pos = 0;
	return bs;
}

bitstream_t* BitStreamCreate(void)
{
	bitstream_t* resultStream = malloc(sizeof(bitstream_t));
	resultStream->buf = malloc(sizeof(unsigned));
	resultStream->capacity = 1;
	resultStream->pos = 0;
	resultStream->length = 0;
	return resultStream;
}

void BitStreamDestroy(bitstream_t* bs)
{
	free(bs->buf);
	free(bs);
}

boolean_t BitStreamAppendBit(bitstream_t* bs, int bit)
{
	int i, k;
	int bitsPerBlock = CHAR_BIT * sizeof(unsigned);

	assert(bs->length <= bs->capacity * bitsPerBlock);
	if (bs->length == bs->capacity * bitsPerBlock)
	{
		int newSize = bs->capacity + 1;
		unsigned *s = realloc(bs->buf, newSize * sizeof(unsigned));
		if (s)
		{
			bs->buf = s;
			bs->capacity = newSize;
		}
		else
		{
			return FALSE;
		}
		return TRUE;
	}

	k = bs->length / bitsPerBlock;
	i = bitsPerBlock - bs->length % bitsPerBlock - 1;
	bs->buf[k] = (bs->buf[k] & ~(1U << i)) | (bit << i);
	bs->length++;
	return TRUE;
}

boolean_t BitStreamAppendStream(bitstream_t* bs, bitstream_t const* other)
{
	unsigned *s = NULL;
	size_t i = 0;
	int bitsPerBlock = sizeof(unsigned) * CHAR_BIT;
	if (bs->length + other->length >= bs->capacity * bitsPerBlock)
	{
		int newSize = (bs->length + other->length + bitsPerBlock - 1) / bitsPerBlock * bitsPerBlock / CHAR_BIT;
		s = realloc(bs->buf, newSize);
		if (s)
		{
			bs->buf = s;
			bs->capacity = newSize / sizeof(unsigned);
		}
		else
		{
			return FALSE;
		}
	}

	for (i = 0; i < other->length; i++) {
		unsigned int block = i / bitsPerBlock;
		int bit = bitsPerBlock - i % bitsPerBlock - 1;
		int value = (other->buf[block] >> bit) & 1;

		block = bs->length / bitsPerBlock;
		bit = bitsPerBlock - bs->length % bitsPerBlock - 1;
		bs->buf[block] = (bs->buf[block] & ~(1U << bit)) | (value << bit);
		bs->length++;
	}
	return TRUE;
}

void BitStreamPrint(bitstream_t* bs)
{
	size_t i;
	int numOfBits = CHAR_BIT * sizeof(unsigned);
	for (i = 0; i < bs->length; i++) {
		unsigned int block = i / numOfBits;
		int bit = numOfBits - i % numOfBits - 1;
		putchar('0' + ((bs->buf[block] >> bit) & 1));
	}
}

void BitStreamSaveToFile(bitstream_t* bs, FILE *f)
{
	unsigned int size = bs->capacity * sizeof(unsigned);
	fwrite(&size, sizeof(size), 1, f);
	fwrite(&bs->length, sizeof(bs->length), 1, f);
	fwrite(bs->buf, size, 1, f);
}

int BitStreamGetLength(bitstream_t* bs)
{
	return bs->length;
}

boolean_t BitStreamResize(bitstream_t* bs, size_t length)
{
	if (bs->length >= length)
	{
		bs->length = length;
		return TRUE;
	}
	else
	{
		unsigned *s = realloc(bs->buf, bs->capacity + 1);
		if (s)
		{
			bs->buf = s;
			bs->capacity++;
		}
		else
		{
			return FALSE;
		}
		bs->length = length;
		return TRUE;
	}
	return FALSE;
}

bitstream_t* BitStreamCreateCopy(bitstream_t* bs)
{
	bitstream_t* copy = malloc(sizeof(bitstream_t));
	copy->length = bs->length;
	copy->buf = malloc(bs->capacity * sizeof(unsigned));
	memcpy(copy->buf, bs->buf, bs->capacity * sizeof(unsigned));
	copy->capacity = bs->capacity;
	copy->pos = bs->pos;
	return copy;
}

int BitStreamGetBit(bitstream_t *bs)
{
	if (bs->pos == bs->length)
	{
		return -1;
	}
	else
	{
		unsigned int block = bs->pos / (CHAR_BIT * sizeof(unsigned));
		int numOfBits = CHAR_BIT * sizeof(unsigned);
		int bit = numOfBits - bs->pos % numOfBits - 1;
		bs->pos++;
		return (bs->buf[block] & (1 << bit)) > 0;
	}
}


#ifdef TEST

int main(void)
{
  bitstream_t* bs = BitStreamCreate();
  bitstream_t* bs2;
  BitStreamAppendBit(bs, 0);
  BitStreamAppendBit(bs, 1);
  BitStreamAppendBit(bs, 1);
  BitStreamAppendBit(bs, 0);
  BitStreamAppendBit(bs, 1);
  BitStreamAppendBit(bs, 0);
  BitStreamPrint(bs);
  putchar('\n');
  bs2 = BitStreamCreateCopy(bs);
  BitStreamPrint(bs2);
  putchar('\n');
  BitStreamAppendStream(bs, bs2);
  BitStreamAppendStream(bs, bs2);
  BitStreamPrint(bs);
  putchar('\n');
  return 0;
}

#endif // TEST
