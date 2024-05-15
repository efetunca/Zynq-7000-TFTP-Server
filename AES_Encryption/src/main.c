/*
 ============================================================================
 Name        : Boot File Encryptor
 Author      : Efe Tunca
 Version     : v1.0.0
 Description : Encrypting Zynq-7000 boot file with AES-256.
 	 	 	   Based on tiny-AES-c by kokke. https://github.com/kokke/tiny-AES-c
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "aes.h"
#include "file_ops.h"

#define ENCRYPTED_FILE_NAME	"BOOT_encrypted.BIN"

int main(void)
{
	char filePath[300] = {0};
	char newFilePath[300] = {0};

	printf("=============== Zynq-7000 TFTP Server ===============\r\n");
	printf("============= Boot File Encryptor v1.0.0 ============\r\n\n");

	printf("Drag and drop the boot file here and then\r\npress enter to start encryption: ");
	scanf(" %[^\n]", filePath);
	if (*filePath == '"') {
		memcpy(filePath, filePath+1, strlen(filePath)-1);
		filePath[strlen(filePath)-2] = '\0';
	}

	char *lastSlash = strrchr(filePath, '\\');
	strncpy(newFilePath, filePath, lastSlash-filePath+1);
	strcat(newFilePath, ENCRYPTED_FILE_NAME);

	open_and_read_file(filePath);
	encrypt_file(newFilePath);

	printf("Encryption completed!\r\n");
	printf("Encrypted boot file is located in the same folder as the original file.\r\n");
	printf("Press enter to exit...");
	while(getchar() != '\n');
	getchar();
}
