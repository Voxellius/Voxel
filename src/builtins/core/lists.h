#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);

    voxel_push(executor, voxel_newList(executor->context));
}

void voxel_builtins_core_newListOf(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_newList(executor->context);

    for (voxel_Int i = 0; i < argCount; i++) {
        voxel_Thing* item = voxel_pop(executor);

        if (!item) {
            break;
        }

        voxel_insertIntoList(executor->context, list, 0, item);
    }

    voxel_push(executor, list);
}

void voxel_builtins_core_getListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list) {
        return voxel_pushNull(executor);
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(executor->context, list, index);

    if (VOXEL_IS_ERROR(listItemResult)) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, listItemResult.value);

    voxel_unreferenceThing(executor->context, list);
}

#endif