typedef struct voxel_Buffer {
    voxel_Count size;
    voxel_Byte* value;
} voxel_Buffer;

voxel_Thing* voxel_newBuffer(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_Buffer* buffer = VOXEL_MALLOC(sizeof(voxel_Buffer));

    buffer->size = size;
    buffer->value = VOXEL_MALLOC(size);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BUFFER;

    if (data != VOXEL_NULL) {
        voxel_copy(data, buffer->value, size);
    } else {
        for (voxel_Count i = 0; i < size; i++) {
            buffer->value[i] = 0;
        }
    }

    return thing;
}

void voxel_destroyBuffer(voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    VOXEL_FREE(buffer->value);
    VOXEL_FREE(buffer);
    VOXEL_FREE(thing);
}