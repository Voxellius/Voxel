#ifndef LIBVOXEL_CONFIG_H_
#define LIBVOXEL_CONFIG_H_

#ifndef VOXEL_USE_CUSTOM_TYPES

#include <stdint.h>

#define VOXEL_BOOL int
#define VOXEL_COUNT unsigned int
#define VOXEL_INT_8 int8_t
#define VOXEL_INT_16 int16_t
#define VOXEL_INT_32 int32_t
#define VOXEL_UINT_8 uint8_t
#define VOXEL_UINT_16 uint16_t
#define VOXEL_UINT_32 uint32_t
#define VOXEL_FLOAT _Float32

#endif

#define VOXEL_DEBUG

#define VOXEL_STRING_BLOCK_SIZE 8

#define VOXEL_USE_STDLIB

#ifdef VOXEL_USE_STDLIB

#include <stdio.h>
#include <stdlib.h>

void _voxel_log_stdlib(char* text) {
    printf(text);
}

void _voxel_errorMessage_stdlib(const char* message, const char* detail, const char* function, const char* file, const int line) {
    printf("%s%s (%s in %s:%d)\n", message, detail, function, file, line);
}

void* _voxel_malloc(VOXEL_COUNT size) {
    return malloc(size);
}

void* _voxel_realloc(void* ptr, VOXEL_COUNT size) {
    return realloc(ptr, size);
}

void _voxel_free(void* ptr) {
    free(ptr);
}

#define VOXEL_LOG _voxel_log_stdlib
#define VOXEL_ERROR_MESSAGE _voxel_errorMessage_stdlib
#define VOXEL_MALLOC _voxel_malloc
#define VOXEL_REALLOC _voxel_realloc
#define VOXEL_FREE _voxel_free

#endif

#endif