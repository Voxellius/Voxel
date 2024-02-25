void voxel_push(voxel_Executor* executor, voxel_Thing* thing) {
    voxel_pushOntoList(executor->context, executor->valueStack, thing);
}

void voxel_pushNull(voxel_Executor* executor) {
    voxel_push(executor, voxel_newNull(executor->context));
}

voxel_Thing* voxel_pop(voxel_Executor* executor) {
    VOXEL_ERRORABLE result = voxel_popFromList(executor->context, executor->valueStack);

    if (VOXEL_IS_ERROR(result)) {
        return VOXEL_NULL;
    }

    return result.value;
}