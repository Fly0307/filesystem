//
// Created by 赵熙 on 2024/3/12.
//

#include <stdlib.h>
#include "file.h"

uint8_t find_min_available_number(uint8_t bitmap[], int size) {
    //返回当前最小可用编号
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 8; j++) {
            if ((bitmap[i] & (1 << j)) == 0) {
                bitmap[i] |= (1 << j);
                return i * 8 + j;
            }
        }
    }

    return -1; // No available number found
}

void clear_bitmap_bit(uint8_t bitmap[], int bit_number) {
    int index = bit_number / 8;
    int offset = bit_number % 8;

    bitmap[index] &= ~(1 << offset);
}

void int_to_string(int num, char fileoid[]) {
    snprintf(fileoid, TEE_OBJECT_ID_MAX_LEN, "%d", num);
}

int createNewFile(const char* dirFilePath, FileEntry* fileEntry){
    FILE* file = fopen(dirFilePath, "rb+");
    if (!file) {
        // 尝试创建一个空的目录文件，因为假设文件不存在
        file = fopen(dirFilePath, "wb+");
        if (!file) {
            perror("Failed to create a new directory file");
            return -1; // 创建文件失败
        }
        uint8_t bitmap_ze[BITMAP_SIZE] = {0};

        fwrite(bitmap_ze, sizeof(uint8_t), BITMAP_SIZE, file);
        fclose(file); // 关闭文件，因为我们只是想创建它
//        return 0; // 成功创建空文件
        fopen(dirFilePath, "rb+");
    }

    int file_num = 0;

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);

    //前32byte作为bitmap存储文件编号
    fseek(file, 0, SEEK_SET);
    uint8_t bitmap[BITMAP_SIZE];
    fread(bitmap,sizeof(uint8_t),BITMAP_SIZE,file);
    file_num = find_min_available_number(bitmap, BITMAP_SIZE);
    if (file_num < 0)
    {
        fclose(file);
        printf("file is too much\n");
        return -1;
    }
    //更新bitmap
    fseek(file, 0, SEEK_SET);
    // 写入BITMAP_SIZE个字节到文件
    fwrite(bitmap, sizeof(uint8_t), BITMAP_SIZE, file);
    fclose(file);

//    memset(fileEntry,0,sizeof( fileEntry));
//    memcpy(fileEntry->fileoid,&file_num,sizeof(file_num));
    fileEntry->filenum = file_num;
//    int_to_string(file_num,fileEntry->fileoid);
    fileEntry->fileoid = file_num;
    char* filePath = malloc(FILE_PATH_LEN);
    memset(filePath, 0 ,FILE_PATH_LEN);
    sprintf(filePath, "%s%u", FILE_PATH, file_num);
    memcpy(fileEntry->filepath,filePath, strlen(filePath));
    file = fopen(fileEntry->filepath, "wb+");
    fclose(file);
}

FILE* openFile(const char* filePath, const char* mode) {
    return fopen(filePath, mode);
}

int writeFile(const char* filePath, const void* data, size_t dataSize) {
    FILE* file = openFile(filePath, "wb+");
    if (!file) return -1;
    char key = 'A'; // 加密密钥
    encryptDecryptData(data, dataSize, key);

    size_t written = fwrite(data, 1, dataSize, file);
    fclose(file);
    return (written == dataSize) ? 0 : -1;
}

int readFile(const char* filePath, const void* data, size_t *dataSize){
    FILE* file = openFile(filePath, "rb");
    if (!file) return -1;
    char key = 'A'; // 解密密钥

    size_t readbytes = fread(data,sizeof(char),256,file );
    *dataSize = readbytes;
    encryptDecryptData(data, *dataSize, key);
    fclose(file);

    return (readbytes == *dataSize) ? 0 : -1;
}

void encryptDecryptData(void* data, size_t dataSize, char key) {
    char* byteData = (char*)data;
    for (size_t i = 0; i < dataSize; ++i) {
        byteData[i] ^= key;
//        char c = byteData[i] ;
//        byteData[i] = c + 1;
    }
}


int readDirFile(const char* dirFilePath, struct dirfile_entry** entries, int* count) {
    FILE* file = fopen(dirFilePath, "rb");
    if (!file) {
        // 尝试创建一个空的目录文件，因为假设文件不存在
        file = fopen(dirFilePath, "wb");
        if (!file) {
            perror("Failed to create a new directory file");
            return -1; // 创建文件失败
        }
        fclose(file); // 关闭文件，因为我们只是想创建它
        *entries = NULL; // 没有条目
        *count = 0; // 条目数量为0
        return 0; // 成功创建空文件
    }

//    if (!file) {
//        perror("Failed to open directory file");
//        return -1;
//    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);

    //前32byte作为bitmap存储文件编号
    fseek(file, 0, SEEK_SET);
    uint8_t bitmap[BITMAP_SIZE];
    fread(bitmap,sizeof(uint8_t),BITMAP_SIZE,file);

    // 计算条目数
    *count = (fileSize - BITMAP_SIZE) / sizeof(struct dirfile_entry);
    if (*count <= 0) {
        fclose(file);
        return 0; // 没有条目
    }

    // 分配内存
    *entries = (struct dirfile_entry*)malloc(*count * sizeof(struct dirfile_entry));
    if (!*entries) {
        perror("Memory allocation failed");
        fclose(file);
        return -1;
    }

    // 读取条目
    size_t readCount = fread(*entries, sizeof(struct dirfile_entry), *count, file);
    if (readCount != *count) {
        perror("Failed to read directory entries");
        free(*entries);
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}



int writeDirFile(const char* dirFilePath, const struct dirfile_entry* entries, int count) {
    FILE* file = fopen(dirFilePath, "rb+");
    if (!file) {
        perror("Failed to open directory file for writing");
        return -1;
    }
    uint8_t bitmap[BITMAP_SIZE];
    if (BITMAP_SIZE != fread(bitmap,sizeof (uint8_t),BITMAP_SIZE,file)){
        perror("Read bitmap error");
        fclose(file);
        return -1;
    }
//    fseek(file, BITMAP_SIZE, SEEK_SET);
    // 写入条目
    size_t writtenCount = fwrite(entries, sizeof(struct dirfile_entry), count, file);
    if (writtenCount != count) {
        perror("Failed to write directory entries");
        fclose(file);
        return -1;
    }


    fclose(file);
    file = fopen(dirFilePath, "rb");
//    uint8_t bitmap[BITMAP_SIZE];
    fread(bitmap,sizeof (uint8_t),BITMAP_SIZE,file);
    fclose(file);
    return 0;
}

int add_newEntry(const char* dirFilePath,const struct dirfile_entry *newEntry){
    struct dirfile_entry *entries = NULL;
    int count = 0;
    if (readDirFile(DIR_FILE_PATH, &entries, &count) != 0) {
        printf("Failed to read directory file.\n");
        return -1;
    }

    // 添加新条目到数组中
//    struct dirfile_entry *updatedEntries = (struct dirfile_entry *) realloc(entries,
//                                                                            (count + 1) * sizeof(struct dirfile_entry));
    struct  dirfile_entry* updatedEntries = (struct dirfile_entry *) malloc((count+1)*sizeof (struct dirfile_entry));
    memmove(updatedEntries, entries, count * sizeof (struct dirfile_entry));
    if (!updatedEntries) {
        printf("Failed to allocate memory for updated entries.\n");
        free(entries);
        return -1;
    }
    updatedEntries[count] = *newEntry;

    // 写回目录文件
    if (writeDirFile(DIR_FILE_PATH, updatedEntries, count + 1) != 0) {
        printf("Failed to update directory file.\n");
        free(updatedEntries);
        return -1;
    }

    free(updatedEntries);
    return 0;
}

int findFile(const char* dirPath, const uint32_t fileoid, FileEntry* foundFile){
    //读取目录索引
//    struct dirfile_entry *r_entries = NULL;
//    int r_count = 0;
//    if (readDirFile(DIR_FILE_PATH, &r_entries, &r_count) != 0) {
//        printf("Failed to read directory file.\n");
//        return -1;
//    }
//    //遍历查找文件
//
//    struct FileEntry * fileEntry = NULL;
//    fileEntry = malloc(sizeof( FileEntry));
//
//    for (int i = 0; i < r_count; ++i) {
//
//    }
    FILE* file = fopen(dirPath, "rb");
    if (!file) {
        perror("Failed to open directory file");
        return -1;
    }

    uint8_t bitmap[BITMAP_SIZE];
    fread(bitmap,sizeof (uint8_t),BITMAP_SIZE,file);
//    fseek(file, BITMAP_SIZE, SEEK_SET);
    struct dirfile_entry entry;
    int found = 0;
    while (fread(&entry, sizeof(struct dirfile_entry), 1, file)) {
        if (entry.oid == fileoid) {
            // 找到了匹配的文件项
            FileEntry fileEntry;
            char* filePath = malloc(FILE_PATH_LEN);
            memset(filePath, 0 , sizeof (FILE_PATH) + 4);
            sprintf(filePath, "%s%u", FILE_PATH, entry.oid);
            memcpy(foundFile->filepath, filePath, strlen(filePath));
            foundFile->fileoid = entry.oid;
            if (foundFile == NULL) {
                perror("Failed to allocate memory for found file");
                fclose(file);
                return -1;
            }
//            memcpy(foundFile->fileoid,fileEntry.fileoid)
//            memcpy(foundFile, &fileEntry, sizeof(FileEntry));
            found = 1;
            break;
        }
    }

    fclose(file);

    if (found) {
        return 0; // 成功找到文件
    } else {
        foundFile = NULL; // 确保指针为空，以避免悬挂指针
        return 1; // 文件未找到
    }

}
