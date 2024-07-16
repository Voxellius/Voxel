#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newWeakRef(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (!thing) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newWeakRef(executor->context, thing));

    voxel_unreferenceThing(executor->context, thing);
}

void voxel_builtins_core_dereferenceWeakRef(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* weakRef = voxel_pop(executor);

    if (!weakRef || weakRef->type != VOXEL_TYPE_WEAK) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_dereferenceWeakRef(executor->context, weakRef));

    voxel_unreferenceThing(executor->context, weakRef);
}

#endif