#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "string.h"
//int main() {
//    printf("Hello, World!\n");
//    return 0;
//}

// 假设UUID、encryptDecryptData、readDirFile、writeDirFile等函数已经定义

//#define DIR_FILE_PATH "./data/dir.db"

void demo() {
    // 假设文件内容和文件名
    char *str = "Hello, encrypted world!";
    char *fileContent = malloc(strlen(str) + 1);
//    char fileName[] = "example_file";
    char key = 'A'; // 加密密钥
    char *filename="example";
    char filePath[256] = "./data/";
    memcpy(fileContent, str, strlen(str));
    // 加密文件内容
    size_t contentSize = strlen(fileContent) + 1; // 包括null终结符
//    encryptDecryptData(fileContent, contentSize, key);
    //拼接获得文件路径
    // 确保filePath以'/'结尾
    if (filePath[strlen(filePath) - 1] != '/') {
        strcat(filePath, "/");
    }
    // 将filename追加到filePath后面
    strcat(filePath, filename);

    FileEntry fileEntry_w;
    createNewFile(DIR_FILE_PATH,&fileEntry_w);

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
//    memset(newEntry.oid, filename, TEE_OBJECT_ID_MAX_LEN);
    memcpy(newEntry.oid,filename, strlen(filename));
    // 假设这里已经正确设置了newEntry的各个字段

    struct dirfile_entry *entries = NULL;
    int count = 0;
    if (readDirFile(DIR_FILE_PATH, &entries, &count) != 0) {
        printf("Failed to read directory file.\n");
        return;
    }

    // 添加新条目到数组中
//    struct dirfile_entry *updatedEntries = (struct dirfile_entry *) realloc(entries,
//                                                                            (count + 1) * sizeof(struct dirfile_entry));
    struct  dirfile_entry* updatedEntries = (struct dirfile_entry *) malloc((count+1)*sizeof (struct dirfile_entry));
    memmove(updatedEntries, entries, count * sizeof (struct dirfile_entry));
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
    uint8_t file_oid[TEE_OBJECT_ID_MAX_LEN];
//    memset(file_oid, 0xaF, TEE_OBJECT_ID_MAX_LEN);
    memcpy(file_oid,filename, strlen(filename));

    FileEntry fileEntry;
    if (findFile(DIR_FILE_PATH, file_oid, &fileEntry))
    {
        printf("don't find file\n");
    }
    else
        printf("find file fileoid=%s\n",fileEntry.fileoid);
    char buffer[256];
    size_t readBytes = 0;
    readFile(fileEntry.filepath,buffer,&readBytes);
    printf("read data:%s \n",buffer);
    // *entries = malloc(sizeof(dirfile_entry) * 读取到的条目数);
    // *count = 读取到的条目数;
    // fread(*entries, sizeof(dirfile_entry), *count, file);
}

int main() {
    demo();
    printf("hello world\n");
    return 0;
}