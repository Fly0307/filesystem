#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "string.h"
//int main() {
//    printf("Hello, World!\n");
//    return 0;
//}

// 假设UUID、encryptDecryptData、readDirFile、writeDirFile等函数已经定义

#define DIR_FILE_PATH "./data/dir.db"

void demo() {
    // 假设文件内容和文件名
    char *str = "Hello, encrypted world!";
    char *fileContent = malloc(strlen(str) + 1);
    char fileName[] = "example_file";
    char key = 'A'; // 加密密钥
    char filePath[256] = "./data/example_file";
    memcpy(fileContent, str, strlen(str));
    // 加密文件内容
    size_t contentSize = strlen(fileContent) + 1; // 包括null终结符
    encryptDecryptData(fileContent, contentSize, key);

    // 写入加密文件内容
    if (writeFile(filePath, fileContent, contentSize) != 0) {
        printf("Failed to write encrypted file.\n");
        return;
    }

    // 更新目录文件
    struct dirfile_entry newEntry;
    // 填充newEntry的字段，例如uuid、oid等
    newEntry.file_number = 0;
    memset(newEntry.hash, 0xFa, TEE_FS_HTREE_HASH_SIZE);
    newEntry.oidlen = 1;
    memset(newEntry.uuid, 0xcf, 16);
    memset(newEntry.oid, 0xaF, TEE_OBJECT_ID_MAX_LEN);
    // 假设这里已经正确设置了newEntry的各个字段

    struct dirfile_entry *entries = NULL;
    int count = 0;
    if (readDirFile(DIR_FILE_PATH, &entries, &count) != 0) {
        printf("Failed to read directory file.\n");
        return;
    }

    // 添加新条目到数组中
    struct dirfile_entry *updatedEntries = (struct dirfile_entry *) realloc(entries,
                                                                            (count + 1) * sizeof(struct dirfile_entry));
    if (!updatedEntries) {
        printf("Failed to allocate memory for updated entries.\n");
        free(entries);
        return;
    }
    updatedEntries[count] = newEntry;

    // 写回目录文件
    if (writeDirFile(DIR_FILE_PATH, updatedEntries, count + 1) != 0) {
        printf("Failed to update directory file.\n");
        free(updatedEntries);
        return;
    }

    free(updatedEntries);

    // 以下是读取和解密文件内容的示例，删除文件和更新目录的操作类似
    // 注意：实际操作中，你需要根据实际情况填充代码，处理错误等
    // *entries = malloc(sizeof(dirfile_entry) * 读取到的条目数);
    // *count = 读取到的条目数;
    // fread(*entries, sizeof(dirfile_entry), *count, file);
}

int main() {
    demo();
    return 0;
}