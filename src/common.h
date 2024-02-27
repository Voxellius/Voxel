typedef VOXEL_BOOL voxel_Bool;
typedef VOXEL_COUNT voxel_Count;
typedef char voxel_Byte;
typedef VOXEL_INT_8 voxel_Int8;
typedef VOXEL_INT_16 voxel_Int16;
typedef VOXEL_INT_32 voxel_Int32;
typedef voxel_Int32 voxel_Int;
typedef VOXEL_UINT_8 voxel_UInt8;
typedef VOXEL_UINT_16 voxel_UInt16;
typedef VOXEL_UINT_32 voxel_UInt32;
typedef voxel_UInt32 voxel_UInt;
typedef VOXEL_FLOAT voxel_Float;
typedef VOXEL_INTPTR voxel_IntPtr;

#define VOXEL_TRUE 1
#define VOXEL_FALSE 0
#define VOXEL_NULL 0

#define VOXEL_NOOP do {} while (0)

#define VOXEL_INTO_PTR(data, pointer) voxel_copy((voxel_Byte*)&(data), (voxel_Byte*)pointer, sizeof(data))

#ifdef VOXEL_DEBUG
    #define VOXEL_DEBUG_LOG VOXEL_LOG
#else
    #define VOXEL_DEBUG_LOG(text) VOXEL_NOOP
#endif

#ifdef VOXEL_DEBUG_THINGS
    #include <stdio.h>

    voxel_Count _voxel_thingCount = 0;

    #define VOXEL_TAG_NEW_THING(type) printf("[New thing %s = %d]\n", #type, ++_voxel_thingCount);
    #define VOXEL_TAG_DESTROY_THING(type) printf("[Destroy thing %s = %d]\n", #type, --_voxel_thingCount);
#else
    #define VOXEL_TAG_NEW_THING(type) VOXEL_NOOP
    #define VOXEL_TAG_DESTROY_THING(type) VOXEL_NOOP
#endif

#ifdef VOXEL_DEBUG_MEMORY
    #include <stdio.h>

    voxel_Count _voxel_memoryUsage = 0;

    #define VOXEL_TAG_MALLOC(type) printf("[Allocate memory for %s, +%d = %d]\n", #type, sizeof(type), _voxel_memoryUsage += sizeof(type))
    #define VOXEL_TAG_MALLOC_SIZE(name, size) printf("[Allocate memory for %s, +%d = %d]\n", name, size, _voxel_memoryUsage += size)
    #define VOXEL_TAG_FREE(type) printf("[Free memory for %s, -%d = %d]\n", #type, sizeof(type), _voxel_memoryUsage -= sizeof(type))
    #define VOXEL_TAG_FREE_SIZE(name, size) printf("[Free memory for %s, -%d = %d]\n", name, size, _voxel_memoryUsage -= size)
    #define VOXEL_TAG_REALLOC(name, oldSize, newSize) printf("[Reallocate memory for %s, -%d +%d = %d]\n", name, oldSize, newSize, _voxel_memoryUsage += newSize - oldSize)
#else
    #define VOXEL_TAG_MALLOC(type) VOXEL_NOOP
    #define VOXEL_TAG_MALLOC_SIZE(name, size) VOXEL_NOOP
    #define VOXEL_TAG_FREE(type) VOXEL_NOOP
    #define VOXEL_TAG_FREE_SIZE(name, size) VOXEL_NOOP
    #define VOXEL_TAG_REALLOC(name, oldSize, newSize) VOXEL_NOOP
#endif

void voxel_copy(voxel_Byte* source, voxel_Byte* destination, voxel_Count size) {
    for (voxel_Count i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

voxel_Bool voxel_compare(voxel_Byte* a, voxel_Byte* b, voxel_Count aSize, voxel_Count bSize) {
    if (aSize != bSize) {
        return VOXEL_FALSE;
    }

    for (voxel_Count i = 0; i < aSize; i++) {
        if (a[i] != b[i]) {
            return VOXEL_FALSE;
        }
    }

    return VOXEL_TRUE;
}