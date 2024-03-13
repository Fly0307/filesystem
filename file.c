//
// Created by 赵熙 on 2024/3/12.
//

#include <stdlib.h>
#include "file.h"

FILE* openFile(const char* filePath, const char* mode) {
    return fopen(filePath, mode);
}

int writeFile(const char* filePath, const void* data, size_t dataSize, FileEntry** fileEntry) {
    FILE* file = openFile(filePath, "wb");
    if (!file) return -1;
    size_t written = fwrite(data, 1, dataSize, file);
    fclose(file);
    return (written == dataSize) ? 0 : -1;
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
    fseek(file, 0, SEEK_SET);

    // 计算条目数
    *count = fileSize / sizeof(struct dirfile_entry);
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
    FILE* file = fopen(dirFilePath, "wb");
    if (!file) {
        perror("Failed to open directory file for writing");
        return -1;
    }

    // 写入条目
    size_t writtenCount = fwrite(entries, sizeof(struct dirfile_entry), count, file);
    if (writtenCount != count) {
        perror("Failed to write directory entries");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int findFile(const char* dirPath, const uint8_t* fileoid, FileEntry** foundFile){
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

    FileEntry entry;
    int found = 0;
    while (fread(&entry, sizeof(FileEntry), 1, file)) {
        if (strcmp(entry.fileoid, fileoid) == 0) {
            // 找到了匹配的文件项
            *foundFile = malloc(sizeof(FileEntry));
            if (*foundFile == NULL) {
                perror("Failed to allocate memory for found file");
                fclose(file);
                return -1;
            }
            memcpy(*foundFile, &entry, sizeof(FileEntry));
            found = 1;
            break;
        }
    }

    fclose(file);

    if (found) {
        return 0; // 成功找到文件
    } else {
        *foundFile = NULL; // 确保指针为空，以避免悬挂指针
        return 1; // 文件未找到
    }

}
