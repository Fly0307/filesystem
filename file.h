//
// Created by 赵熙 on 2024/3/12.
//

#ifndef FILESYSTEM_FILE_H
#define FILESYSTEM_FILE_H

#include <stdio.h>
#include <stdint.h>
#include <sys/dtrace.h>
#include "string.h"

#define DIR_FILE_PATH "./data/dir.db"

#define TEE_OBJECT_ID_MAX_LEN 16
#define TEE_FS_HTREE_HASH_SIZE 256/8

typedef struct {
    char fileoid[TEE_OBJECT_ID_MAX_LEN]; // 文件名
    char filepath[64];  //文件路径
} FileEntry;

struct dirfile_entry {
    UUID uuid;//创建该文件的UUID
    uint8_t oid[TEE_OBJECT_ID_MAX_LEN];//安全文件的名字（使用secure storage操作时的名字）
    uint32_t oidlen;//文件名字的长度
    uint8_t hash[TEE_FS_HTREE_HASH_SIZE];//data/目录下安全文件的root node的hash值
    uint32_t file_number;//保存在/data/目录下的文件编号
};

// 读取目录文件
//int readDir(const char* dirPath, FileEntry** fileList, int* fileCount);
int readDirFile(const char* dirFilePath, struct dirfile_entry** entries, int* count);

//写入目录文件
int writeDirFile(const char* dirFilePath, const struct dirfile_entry* entries, int count);

// 打开指定文件
FILE* openFile(const char* filePath, const char* mode);

// 写入指定文件
int writeFile(const char* filePath, const void* data, size_t dataSize, FileEntry** fileEntry);

// 查找指定文件
int findFile(const char* dirPath, const uint8_t* fileName, FileEntry** foundFile);

// 删除指定文件
int deleteFile(const char* filePath);

// 加密数据
void encryptData(void* data, size_t dataSize, char key);

// 解密数据
void decryptData(void* data, size_t dataSize, char key);

void encryptDecryptData(void* data, size_t dataSize, char key);

void get_file_hash();

// 加密和解密可以使用同一个函数，因为异或操作是可逆的
#define encryptData(data, dataSize, key) encryptDecryptData((data), (dataSize), (key))
#define decryptData(data, dataSize, key) encryptDecryptData((data), (dataSize), (key))


#endif //FILESYSTEM_FILE_H
