//
// Created by 赵熙 on 2024/3/12.
//

#include <stdlib.h>
#include <stdbool.h>
#include "file.h"

// 判断两个UUID是否相等
bool compare_uuid(UUID uuid1, UUID uuid2) {
    if (uuid1.data1 != uuid2.data1) {
        return false;
    }

    if (uuid1.data2 != uuid2.data2) {
        return false;
    }

    if (uuid1.data3 != uuid2.data3) {
        return false;
    }

    for (int i = 0; i < 8; i++) {
        if (uuid1.data4[i] != uuid2.data4[i]) {
            return false;
        }
    }

    return true;
}

uint8_t find_min_available_number(uint8_t bitmap[], int size) {
    //返回当前最小可用编号
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 8; j++) {
            if ((bitmap[i] & (1 << j)) == 0) {
                bitmap[i] |= (1 << j);
                return (i * 8 + j)+1;
            }
        }
    }

    return -1; // No available number found
}

void clear_bit(uint8_t bitmap[],int position){
    int byte_index = position / 8;
    int bit_offset = position % 8;
    bitmap[byte_index] &= ~(1 << bit_offset);
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

int deleteDirEntry(const char* dirFilePath, char* filename){
    struct dirfile_entry *entries = NULL;
    int count = 0;
    uint8_t bitmap[BITMAP_SIZE];
    if (readDirFile(DIR_FILE_PATH, &entries, &count, bitmap) != 0) {
        printf("Failed to read directory file.\n");
        return -1;
    }

    //遍历条目，查找到对应的fileentry
    int find_id = -1;
    for (int i = 0; i < count; ++i) {
        if (strcmp(entries[i].filename, filename) == 0){
            find_id = i;
            break;
        }
    }
    if (find_id >= 0){
        clear_bit(bitmap, find_id);
    }
    char* filePath = malloc(FILE_PATH_LEN);
    memset(filePath, 0 , sizeof (FILE_PATH) + 4);
    sprintf(filePath, "%s%u", FILE_PATH, entries[find_id].oid);
    //删除指定文件
    if(remove((filePath)) == 0){
        printf(" file %s is deteted\n", filePath);
    } else{
        printf("delete file %s failed!\n", filePath);
    }

    //拷贝后面的数据，删除当前entry
//    for (int i = find_id; i < (count -1); ++i) {
//        memcpy(entries+i , entries+(i+1), sizeof(struct dirfile_entry));
//    }
    struct  dirfile_entry* updatedEntries = (struct dirfile_entry *) malloc((count-1)*sizeof (struct dirfile_entry));

    if (find_id > 0){
        //    拷贝foundid前的数据
        memmove(updatedEntries, entries, find_id * sizeof (struct dirfile_entry));
        //    拷贝foundid后的数据
        memmove(updatedEntries + find_id, entries + (find_id + 1), ((count - find_id -1)) *sizeof(struct dirfile_entry));
    } else if (find_id == 0 && count >1){
        memmove(updatedEntries, entries + 1, (count-1) *sizeof(struct dirfile_entry));
    }

    if (!updatedEntries) {
        printf("Failed to allocate memory for updated entries.\n");
        free(entries);
        return -1;
    }
    // 写回目录文件
    if (writeDirFile(DIR_FILE_PATH, updatedEntries, count - 1, bitmap) != 0) {
        printf("Failed to update directory file.\n");
        free(entries);
        free(updatedEntries);
        return -1;
    }

    free(entries);
    free(updatedEntries);

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
//    char* byteData = (char*)data;
//    for (size_t i = 0; i < dataSize; ++i) {
//        byteData[i] ^= key;
////        char c = byteData[i] ;
////        byteData[i] = c + 1;
//    }
}


int readDirFile(const char* dirFilePath, struct dirfile_entry** entries, int* count, uint8_t* bitmap) {
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
//    uint8_t bitmap[BITMAP_SIZE];
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



int writeDirFile(const char* dirFilePath, const struct dirfile_entry* entries, int count, uint8_t* bitmap) {
    FILE* file = fopen(dirFilePath, "wb+");
    if (!file) {
        perror("Failed to open directory file for writing");
        return -1;
    }
//    uint8_t old_bitmap[BITMAP_SIZE];
    if (bitmap != NULL){
        if (BITMAP_SIZE != fwrite(bitmap,sizeof (uint8_t),BITMAP_SIZE,file)){
            perror("write bitmap error");
            fclose(file);
            return -1;
        }
    } else {
        fseek(file,BITMAP_SIZE,SEEK_SET);
        perror("bitmap is NULL");
        return -1;
    }
//    fseek(file, BITMAP_SIZE, SEEK_SET);
    // 写入条目
    if (count == 0){
//        entries = malloc(sizeof (struct dirfile_entry));
//        memset(entries, 0 ,sizeof(struct dirfile_entry));
//        size_t writtenCount = fwrite(entries, sizeof(struct dirfile_entry), 1, file);
//        if (writtenCount != 1) {
//            perror("Failed to write directory entries");
//            fclose(file);
//            free(entries);
//            return -1;
//        }
//        free(entries);
    } else{
        size_t writtenCount = fwrite(entries, sizeof(struct dirfile_entry), count, file);
        if (writtenCount != count) {
            perror("Failed to write directory entries");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    // for debug
    file = fopen(dirFilePath, "rb");
    fread(bitmap,sizeof (uint8_t),BITMAP_SIZE,file);
    fclose(file);
    return 0;
}

int add_newEntry(const char* dirFilePath,const struct dirfile_entry *newEntry){
    struct dirfile_entry *entries = NULL;
    int count = 0;
    uint8_t bitmap[BITMAP_SIZE];
    if (readDirFile(DIR_FILE_PATH, &entries, &count, bitmap) != 0) {
        printf("Failed to read directory file.\n");
        return -1;
    }

    // 添加或者插入新条目到数组中,考虑count为1时是否为空
    // 当前count>bitmap，直接插入到对应位置
//    struct dirfile_entry *updatedEntries = (struct dirfile_entry *) realloc(entries,
//                                                                            (count + 1) * sizeof(struct dirfile_entry));
    if (count > newEntry->oid){
        //插入到现有entries对应位置
        memcpy(entries + newEntry->oid, newEntry, sizeof (struct dirfile_entry));
        // 写回目录文件
        if (writeDirFile(DIR_FILE_PATH, entries, count, bitmap) != 0) {
            printf("Failed to update directory file.\n");
            free(entries);
            return -1;
        }
    } else{
        //需要追加一个新entry
        struct  dirfile_entry* updatedEntries = (struct dirfile_entry *) malloc((count+1)*sizeof (struct dirfile_entry));
        memmove(updatedEntries, entries, count * sizeof (struct dirfile_entry));
        if (!updatedEntries) {
            printf("Failed to allocate memory for updated entries.\n");
            free(entries);
            free(updatedEntries);
            return -1;
        }
        updatedEntries[count] = *newEntry;
        // 写回目录文件
        if (writeDirFile(DIR_FILE_PATH, updatedEntries, count + 1, bitmap) != 0) {
            printf("Failed to update directory file.\n");
            free(entries);
            free(updatedEntries);
            return -1;
        }
        free(updatedEntries);
    }

    free(entries);
    return 0;
}

/**
 *  返回1则未找到文件
 * @param dirPath
 * @param fileoid
 * @param foundFile
 * @return 1 0
 */
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
        if (entry.oid == fileoid && entry.namelen > 0) {
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

int deleteFile(const char* filePath){

}
