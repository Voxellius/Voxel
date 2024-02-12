#ifndef LIBVOXEL_CONFIG_H_
#define LIBVOXEL_CONFIG_H_

#define VOXEL_COUNT unsigned int

#define VOXEL_USE_STDLIB

#ifdef VOXEL_USE_STDLIB

#include <stdio.h>
#include <stdlib.h>

void _voxel_log_stdlib(char* text) {
    printf(text);
}

void* _voxel_malloc(VOXEL_COUNT size) {
    return malloc(size);
}

void _voxel_free(void* ptr) {
    free(ptr);
}

#define VOXEL_LOG _voxel_log_stdlib
#define VOXEL_MALLOC _voxel_malloc
#define VOXEL_FREE _voxel_free

#endif

#endif