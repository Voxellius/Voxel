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

#define VOXEL_TRUE 1
#define VOXEL_FALSE 0
#define VOXEL_NULL 0

#define VOXEL_INTO_PTR(data, pointer) voxel_copy((voxel_Byte*)&(data), (voxel_Byte*)pointer, sizeof(data))

#ifdef VOXEL_DEBUG
    #define VOXEL_DEBUG_LOG VOXEL_LOG
#else
    void VOXEL_DEBUG_LOG(char* text) {}
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