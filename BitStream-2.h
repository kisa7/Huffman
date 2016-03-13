#ifndef BITSTREAM_INCLUDED
#define BITSTREAM_INCLUDED
#include <stdio.h>

typedef struct bitstream_t bitstream_t;
typedef enum
{
	FALSE,
	TRUE
}boolean_t;

bitstream_t* BitStreamCreate(void);
bitstream_t* BitStreamCreateFromFile(FILE *f);
bitstream_t* BitStreamCreateCopy(bitstream_t* bs);
void BitStreamDestroy(bitstream_t *bs);

boolean_t BitStreamAppendBit(bitstream_t* bs, int bit);
boolean_t BitStreamAppendStream(bitstream_t *bs, bitstream_t const *other);

void BitStreamPrint(bitstream_t *bs);
void BitStreamSaveToFile(bitstream_t *bs, FILE *f);

int BitStreamGetLength(bitstream_t *bs);
int BitStreamGetBit(bitstream_t *bs);
boolean_t BitStreamResize(bitstream_t *bs, size_t length);

#endif