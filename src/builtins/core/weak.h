#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newWeakRef(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* thing = voxel_pop(executor);

    VOXEL_REQUIRE(thing);

    voxel_push(executor, voxel_newWeakRef(executor->context, thing));

    voxel_finally:

    voxel_unreferenceThing(executor->context, thing);
}

void voxel_builtins_core_dereferenceWeakRef(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* weakRef = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(weakRef, VOXEL_TYPE_WEAK));

    voxel_push(executor, voxel_dereferenceWeakRef(executor->context, weakRef));

    voxel_finally:

    voxel_unreferenceThing(executor->context, weakRef);
}

#endif