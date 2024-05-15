/*
 * file_ops.c
 *
 *  Created on: 13 May 2024
 *      Author: Efe Tunca
 */

#include <stdio.h>
#include <stdlib.h>
#include "aes.h"

uint8_t key[] 	= { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
					0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
uint8_t iv[]  	= { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

long fileSize = 0;
unsigned char *fileContent;

void open_and_read_file(const char *filename)
{
	size_t numBytesRead;

	FILE *file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileContent = (unsigned char *)calloc(fileSize, sizeof(unsigned char));

	numBytesRead = fread(fileContent, 1, fileSize, file);
	printf("\r\n%u bytes read\r\n", numBytesRead);

	if (numBytesRead != fileSize)
		printf("\r\nInvalid read file!\r\n");

	fclose(file);
}

int encrypt_file(const char *filename)
{
	struct AES_ctx ctx;
	size_t numBytesWritten;

	FILE *newFile = fopen(filename, "wb");

	aes_init_ctx_iv(&ctx, key, iv);

	int newSize = fileSize;
	if (fileSize % 16)
		newSize += 16 - (fileSize % 16);

	uint8_t *newContent = (uint8_t *)calloc(newSize, sizeof(uint8_t));

	for (int fileContentIndex = 0; fileContentIndex < fileSize; fileContentIndex++)
		newContent[fileContentIndex] = fileContent[fileContentIndex];

	uint8_t padByte = 16 - (fileSize % 16);
	if (fileSize + padByte > newSize)
		return 0;

	for (int padIndex = 0; padIndex < padByte; padIndex++)
		newContent[fileSize + padIndex] = padByte;

	encrypt_aes(&ctx, newContent, newSize);
	numBytesWritten = fwrite(newContent, 1, newSize, newFile);

	printf("%u bytes written\r\n\n", numBytesWritten);

	fclose(newFile);
	free(fileContent);

    return 1;
}
