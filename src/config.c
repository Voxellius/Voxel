#define VOXEL_USE_STDLIB

#ifdef VOXEL_USE_STDLIB

#include <stdio.h>

void _voxel_log_stdlib(char* text) {
    printf(text);
}

#define VOXEL_LOG _voxel_log_stdlib

#endif