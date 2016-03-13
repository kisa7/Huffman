#define _CRTDBG_MAP_ALLOC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <crtdbg.h>
#include <ctype.h>
#include <math.h>
#include "BitStream-2.h"

#pragma warning(disable:4996)

#define SIZE 255

typedef struct huffcode_t
{
	int frequency;
	bitstream_t *huffCode;
}huffcode_t;

typedef struct frequency_tree_t
{
	int ASCIIcode;
	int frequency;
	struct frequency_tree_t *next;
	struct frequency_tree_t *right;
	struct frequency_tree_t *left;
}frequency_tree_t;
typedef struct list_t
{
	frequency_tree_t *first;
}list_t;

void FrequencyCount(huffcode_t* a, char* s)
{
	int i = 0;
	while (s[i] != '\0')
	{
		(a[s[i]].frequency)++;
		i++;
	}
}

void WriteEncryption(huffcode_t* a, bitstream_t* bs)
{
	int i = 0;
	FILE *fMyFile = fopen("ChanceToGetBackYourFiles.bin", "wb");

	BitStreamSaveToFile(bs, fMyFile);

    for (i = 0; i < SIZE; i++)
	{
		if (a[i].frequency)
		{
			fwrite(&i, sizeof(i), 1, fMyFile);
			fwrite(&a[i].frequency, sizeof(a[i].frequency), 1, fMyFile);
		}
	}
	fclose(fMyFile);
}

char* ReadInputString(void)
{
	char *s = NULL;
	int size = 0;
	FILE *fMyFile = fopen("test.txt", "rb");
	if (!fMyFile)
	{
		printf("Aiaiaiai!\n");
		exit(-1);
	}
	fseek(fMyFile, 0, SEEK_END);
	size = ftell(fMyFile);
	s = malloc(size + 1);
	fseek(fMyFile, 0, SEEK_SET);
	fgets(s, (size + 1), fMyFile);
	fclose(fMyFile);
	return s;
}

static void _Print(frequency_tree_t* node, int level)
{
	int i = 0;
	if (node)
	{
		_Print(node->left, level + 1);
		printf("%*s", 4 * level, " ");
		if (node->ASCIIcode > SIZE || !isprint(node->ASCIIcode))
		{
			printf("%i\n", node->ASCIIcode);
		}
		else
		{
			printf("%c\n", node->ASCIIcode);
		}
		_Print(node->right, level + 1);
	}
}
void Print(list_t* list)
{
	_Print(list->first, 0);
}

void PrintHuff(huffcode_t* a)
{
	int i = 0;
	for (i = 0; i < SIZE; i++)
	{
		if (a[i].frequency)
		{
			printf("%c - ", i);
			BitStreamPrint(a[i].huffCode);
			printf("\n");
		}
	}
}


static void _ListInsert(list_t *list, frequency_tree_t* node)
{
	frequency_tree_t head;
	frequency_tree_t* temp = &head;
	head.next = list->first;
	while (temp->next != NULL && node->frequency >= temp->next->frequency)
	{
		temp = temp->next;
	}
	node->next = temp->next;
	temp->next = node;
	list->first = head.next;
}

list_t* _ListCreate()
{
	list_t* list = malloc(sizeof(list_t));
	if (list)
	{
		list->first = NULL;
	}
	return list;
}

list_t* CreateSortedList(huffcode_t* a)
{
	int i = 0;
	list_t* list;
	list = _ListCreate();
	for (i = 0; i < SIZE; i++)
	{
		if (a[i].frequency)
		{
			frequency_tree_t* node = malloc(sizeof(frequency_tree_t));
			if (node)
			{
				node->ASCIIcode = i;
				node->frequency = a[i].frequency;
				node->next = NULL;
				node->left = NULL;
				node->right = NULL;
				_ListInsert(list, node);
			}
		}
	}
	return list;
}

static frequency_tree_t* _SumTwoElem(frequency_tree_t* first, frequency_tree_t* second, int *ASCIICode)
{
	frequency_tree_t* node = malloc(sizeof(frequency_tree_t));
	if (node)
	{
		node->ASCIIcode = ++(*ASCIICode);
		node->frequency = first->frequency + second->frequency;
		node->next = NULL;
		node->left = first;
		node->right = second;
	}
	return node;
}
void CreateTree(list_t *list)
{
	int depth = SIZE;
	while (list->first->next)
	{
		frequency_tree_t* node = _SumTwoElem(list->first, list->first->next, &depth);
		list->first = list->first->next->next;
		_ListInsert(list, node);
	}
}

void Traverse(frequency_tree_t* node, bitstream_t *bs, huffcode_t* a)
{
	if (node->left && node->right)
	{
		BitStreamAppendBit(bs, 0);
		Traverse(node->left, bs, a);
		BitStreamResize(bs, BitStreamGetLength(bs) - 1);

		BitStreamAppendBit(bs, 1);
		Traverse(node->right, bs, a);
		BitStreamResize(bs, BitStreamGetLength(bs) - 1);
	}
	else
	{
		assert(node->ASCIIcode < 256);
		assert(node->left == NULL && node->right == NULL);
		a[node->ASCIIcode].huffCode = BitStreamCreateCopy(bs);
	}
}

void TreeDestroy(frequency_tree_t *node)
{
	if (node)
	{
		TreeDestroy(node->left);
		TreeDestroy(node->right);
		free(node);
	}
}

bitstream_t* Encryption(char* inputString, huffcode_t* a)
{
	int i = 0;
    bitstream_t* bs = BitStreamCreate();
	for (i = 0; inputString[i] != '\0'; i++)
    {
		BitStreamAppendStream(bs, a[inputString[i]].huffCode);
    }
    return bs;
}

void Decryption(bitstream_t *bs, frequency_tree_t* node)
{
	int code = BitStreamGetBit(bs);
	frequency_tree_t* temp = node;
	while (code != -1)
	{
		if (code && node->right)
		{
			node = node->right;
		}
		else if (node->left)
		{
			node = node->left;
		}
		else
		{
			printf("%c", node->ASCIIcode);
			node = temp;
			continue;
		}
		code = BitStreamGetBit(bs);
	}
	printf("%c\n", node->ASCIIcode);
}

void HuffDestroy(huffcode_t* a)
{
	int i = 0;
	for (i = 0; i < SIZE; i++)
	{
		if (a[i].frequency)
		{
			BitStreamDestroy(a[i].huffCode);
		}
	}
}

bitstream_t* ReadDictionary(huffcode_t* a, bitstream_t* bs)
{
	int i = 0;
	FILE *myFile = fopen("ChanceToGetBackYourFiles.bin", "rb");
	bs = BitStreamCreateFromFile(myFile);
	while (1)
	{
		if (fread(&i, sizeof(i), 1, myFile) != 1)
			break;
		fread(&a[i].frequency, sizeof(a[i].frequency), 1, myFile);
	}
	fclose(myFile);
	return bs;
}

int main(void)
{
	int fileSize = 0;
	char *inputString = NULL;
	int size = -1;
	bitstream_t* bs = NULL;
	list_t* list;
	int i = 0;
	int n = 0;
	huffcode_t a[SIZE] = { { 0 } };

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

	printf("1.Encipher or 2.Decipher?");
	scanf("%i", &n);
	if (n == 1)
	{
		inputString = ReadInputString();
		FrequencyCount(a, inputString);
		list = CreateSortedList(a);
		CreateTree(list);
		Print(list);

  		bs = BitStreamCreate();
		Traverse(list->first, bs, a);
		BitStreamDestroy(bs);
		
		PrintHuff(a);

		bs = Encryption(inputString, a);
		WriteEncryption(a, bs);
		BitStreamDestroy(bs);

		TreeDestroy(list->first);
		free(list);
		free(inputString);
		HuffDestroy(a);
	}
	else
	{
		bs = ReadDictionary(a, bs);
		list = CreateSortedList(a);
		CreateTree(list);
		Print(list);
		Decryption(bs, list->first);
		BitStreamDestroy(bs);
		TreeDestroy(list->first);
		free(list);
	}
	return 0;
}