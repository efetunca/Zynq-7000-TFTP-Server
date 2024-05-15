/*
 * file_ops.h
 *
 *  Created on: 13 May 2024
 *      Author: Efe Tunca
 */

#ifndef FILE_OPS_H_
#define FILE_OPS_H_

extern long fileSize;

void open_and_read_file(const char *filename);
int encrypt_file(const char *filename);

#endif /* FILE_OPS_H_ */
