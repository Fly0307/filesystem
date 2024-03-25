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
    char *filename="example_2";
    char filePath[256] = "./data/";
    memcpy(fileContent, str, strlen(str));
    UUID uuid1 = {0x12345678, 0xABCD, 0xEF01, {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0}};
    UUID uuid2 = {0x12345678, 0xABCD, 0xEF01, {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1}};

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
    fileEntry_w.filepath = malloc(FILE_PATH_LEN);
    memset(fileEntry_w.filepath, 0, FILE_PATH_LEN);
    fileEntry_w.filename_sz = strlen(filename);
    fileEntry_w.filename = malloc(fileEntry_w.filename_sz);
    memcpy(fileEntry_w.filename, filename, strlen(filename));
    createNewFile(DIR_FILE_PATH,&fileEntry_w);

    // 写入加密文件内容
    if (writeFile(fileEntry_w.filepath, fileContent, contentSize) != 0) {
        printf("Failed to write encrypted file.\n");
        free(fileContent);
        return;
    }

    // 更新目录文件
    //  这里正确设置了newEntry的各个字段
    struct dirfile_entry newEntry;
    memset(&newEntry, 0, sizeof(struct dirfile_entry));
    // 填充newEntry的字段，例如uuid、oid等
    newEntry.oid = fileEntry_w.fileoid;
    memcpy(newEntry.filename,filename, strlen(filename));
    newEntry.namelen = strlen(newEntry.filename);
    memset(newEntry.hash, 0xFa, TEE_FS_HTREE_HASH_SIZE);
    memcpy(&newEntry.uuid, &uuid1, sizeof (UUID));
//    memset(newEntry.oid, filename, TEE_OBJECT_ID_MAX_LEN);
//    memcpy(newEntry.oid,filename, strlen(filename));

    add_newEntry(DIR_FILE_PATH, &newEntry);

    // 以下是读取和解密文件内容的示例，删除文件和更新目录的操作类似
    // 注意：实际操作中，你需要根据实际情况填充代码，处理错误等
    uint32_t file_oid;
//    memset(file_oid, 0xaF, TEE_OBJECT_ID_MAX_LEN);
    file_oid = fileEntry_w.fileoid;

    FileEntry fileEntry;
    fileEntry.filepath = malloc(FILE_PATH_LEN);
    if (findFile(DIR_FILE_PATH, file_oid, &fileEntry))
    {
        printf("don't find file\n");
    }
    else
        printf("find file fileoid=%u\n",fileEntry.fileoid);
    char buffer[256];
    size_t readBytes = 0;
    readFile(fileEntry.filepath,buffer,&readBytes);
    printf("read data:%s \n",buffer);
    // *entries = malloc(sizeof(dirfile_entry) * 读取到的条目数);
    // *count = 读取到的条目数;
    // fread(*entries, sizeof(dirfile_entry), *count, file);
    deleteDirEntry(DIR_FILE_PATH, filename);
    if (findFile(DIR_FILE_PATH, file_oid, &fileEntry))
    {
        printf("don't find file, delete success\n");
    } else {
        printf("delete file %s failed\n",filename);
    }

    memset(fileEntry_w.filepath, 0, FILE_PATH_LEN);
    createNewFile(DIR_FILE_PATH,&fileEntry_w);

    // 写入加密文件内容
    if (writeFile(fileEntry_w.filepath, fileContent, contentSize) != 0) {
        printf("Failed to write encrypted file.\n");
        free(fileContent);
        return;
    }
    add_newEntry(DIR_FILE_PATH, &newEntry);

    free(fileContent);
    free(fileEntry_w.filepath);
    free(fileEntry_w.filename);
}

int main() {
    demo();
    printf("hello world\n");
    return 0;
}