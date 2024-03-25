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

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
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

void voxel_builtins_core_setListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 3) {
        return;
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return;
    }

    if (VOXEL_IS_ERROR(voxel_setListItem(executor->context, list, index, value))) {
        return;
    }

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_removeListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return voxel_pushNull(executor);
    }

    voxel_removeListItem(executor->context, list, index);

    voxel_unreferenceThing(executor->context, list);

    voxel_pushNull(executor);
}

void voxel_builtins_core_pushOntoList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    if (VOXEL_IS_ERROR(voxel_pushOntoList(executor->context, list, value))) {
        return voxel_pushNull(executor);
    }

    voxel_unreferenceThing(executor->context, list);
    voxel_unreferenceThing(executor->context, value);

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));
}

void voxel_builtins_core_popFromList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE result = voxel_popFromList(executor->context, list);

    if (VOXEL_IS_ERROR(result)) {
        return voxel_pushNull(executor);
    }

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, result.value);
}

void voxel_builtins_core_insertIntoList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 3) {
        return;
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return;
    }

    if (VOXEL_IS_ERROR(voxel_insertIntoList(executor->context, list, index, value))) {
        return;
    }

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_getListLength(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return;
    }

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));
}

void voxel_builtins_core_joinList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* delimeter = voxel_popString(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE result = voxel_joinList(executor->context, list, delimeter);

    if (VOXEL_IS_ERROR(result)) {
        return voxel_pushNull(executor);
    }

    voxel_unreferenceThing(executor->context, list);
    voxel_unreferenceThing(executor->context, delimeter);

    voxel_push(executor, result.value);
}

#endif