/*
 * web_utils.h
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#ifndef SRC_WEB_UTILS_H_
#define SRC_WEB_UTILS_H_

#include "ff.h"
#include "rtc.h"

#define MAX_FOLDER_LEVEL	20
#define MAX_PATH_LENGTH		512
#define MAX_FILE_LENGTH		128

typedef struct {
	DIR dir;
	int level;
	char name[MAX_FILE_LENGTH];
} DIRSTACK;

void listDirectory(const char *path);
void createIndexFileTree(const char *path);
void convertFileSize(char *convertedFileSize, FILINFO *info);
void setTimestamp(char *fname);

#endif /* SRC_WEB_UTILS_H_ */
