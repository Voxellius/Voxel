voxel_Handle* voxel_openHandle(voxel_Context* context, voxel_HandleType* type, void* value) {
    voxel_Handle* handle = (voxel_Handle*)VOXEL_MALLOC(sizeof(voxel_Handle)); VOXEL_TAG_MALLOC(voxel_Handle);

    handle->type = type;
    handle->id = context->nextHandleId++;
    handle->value = value;
    handle->previousHandle = context->lastHandle;
    handle->nextHandle = VOXEL_NULL;

    if (context->lastHandle) {
        context->lastHandle->nextHandle = handle;
    }

    context->lastHandle = handle;

    if (!context->firstHandle) {
        context->firstHandle = handle;
    }

    return handle;
}

void voxel_closeHandle(voxel_Context* context, voxel_Handle* handle) {
    if (handle->type->closer) {
        handle->type->closer(handle);
    }

    if (handle == context->firstHandle) {
        context->firstHandle = handle->nextHandle;
    }

    if (handle == context->lastHandle) {
        context->lastHandle = handle->previousHandle;
    }

    if (handle->previousHandle) {
        handle->previousHandle->nextHandle = handle->nextHandle;
    }

    if (handle->nextHandle) {
        handle->nextHandle->previousHandle = handle->previousHandle;
    }

    VOXEL_FREE(handle); VOXEL_TAG_FREE(voxel_Handle);
}

voxel_Handle* voxel_getHandleById(voxel_Context* context, voxel_Count id) {
    voxel_Handle* currentHandle = context->firstHandle;

    while (currentHandle) {
        if (currentHandle->id == id) {
            return currentHandle;
        }

        currentHandle = currentHandle->nextHandle;
    }

    return VOXEL_NULL;
}