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

voxel_Thing* voxel_popNumber(voxel_Executor* executor) {
    voxel_Thing* poppedThing = voxel_pop(executor);

    if (!poppedThing) {
        return VOXEL_NULL;
    }

    if (poppedThing->type == VOXEL_TYPE_NUMBER) {
        return poppedThing; // Saves us from having to copy the thing
    }

    VOXEL_ERRORABLE result = voxel_thingToNumber(executor->context, poppedThing);

    voxel_unreferenceThing(executor->context, poppedThing);

    if (VOXEL_IS_ERROR(result)) {
        return VOXEL_NULL;
    }

    return result.value;
}