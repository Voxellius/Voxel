#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newList(voxel_Executor* executor) {
    VOXEL_ARGC(0);

    voxel_push(executor, voxel_newList(executor->context));

    voxel_finally:
}

void voxel_builtins_core_newListOf(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_newList(executor->context);

    for (voxel_Count i = 0; i < argCount; i++) {
        voxel_Thing* item = voxel_pop(executor);

        if (!item) {
            break;
        }

        voxel_insertIntoList(executor->context, list, 0, item);
        voxel_unreferenceThing(executor->context, item);
    }

    voxel_push(executor, list);

    voxel_finally:
}

void voxel_builtins_core_getListItem(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST));

    voxel_List* listValue = (voxel_List*)list->value;

    if (index < 0) {
        index = listValue->length + index;
    }

    if (index < 0 || index >= listValue->length) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(executor->context, list, index);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(listItemResult));

    voxel_ListItem* listItem = (voxel_ListItem*)listItemResult.value;

    if (!listItem) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Thing* value = listItem->value;

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_setListItem(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST) && !list->isLocked);

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    VOXEL_ERRORABLE getListItemResult = voxel_getListItem(executor->context, list, index);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(getListItemResult));

    voxel_ListItem* listItem = (voxel_ListItem*)getListItemResult.value;

    if (!listItem) {
        if (index == voxel_getListLength(list)) {
            VOXEL_REQUIRE(!VOXEL_IS_ERROR(voxel_pushOntoList(executor->context, list, value)));

            value->referenceCount++;

            voxel_push(executor, value);
        } else {
            voxel_pushNull(executor);
        }

        goto voxel_finally;
    }

    voxel_Thing* currentValue = listItem->value;

    voxel_unreferenceThing(executor->context, currentValue);

    listItem->value = value;
    value->referenceCount += 2;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_removeListItem(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST) && !list->isLocked);

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_removeListItem(executor->context, list, index);

    voxel_pushNull(executor);

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_pushOntoList(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST) && !list->isLocked);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(voxel_pushOntoList(executor->context, list, value)));

    value->referenceCount--;

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_popFromList(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST) && !list->isLocked);

    voxel_List* listValue = (voxel_List*)list->value;
    voxel_ListItem* lastItem = listValue->lastItem;

    if (!lastItem) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Thing* lastThing = lastItem->value;

    lastThing->referenceCount++;

    VOXEL_ERRORABLE result = voxel_popFromList(executor->context, list);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(result));

    voxel_push(executor, lastThing);

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_insertIntoList(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || list->isLocked) {
        goto voxel_finally;
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        goto voxel_finally;
    }

    if (VOXEL_IS_ERROR(voxel_insertIntoList(executor->context, list, index, value))) {
        goto voxel_finally;
    }

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_getListLength(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST));

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_joinList(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* delimeter = voxel_popString(executor);
    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST));

    VOXEL_ERRORABLE result = voxel_joinList(executor->context, list, delimeter);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(result));

    voxel_push(executor, (voxel_Thing*)result.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, list);
    voxel_unreferenceThing(executor->context, delimeter);
}

void voxel_builtins_core_concatList(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* source = voxel_pop(executor);
    voxel_Thing* destination = voxel_pop(executor);

    VOXEL_REQUIRE(
        VOXEL_ARG(source, VOXEL_TYPE_LIST) &&
        VOXEL_ARG(destination, VOXEL_TYPE_LIST)
    );

    voxel_concatList(executor->context, destination, source);

    voxel_push(executor, destination);

    voxel_finally:

    voxel_unreferenceThing(executor->context, source);
}

void voxel_builtins_core_findListItemIndex(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Bool equality = voxel_popBoolean(executor);
    voxel_Thing* item = voxel_pop(executor);
    voxel_Thing* list = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(list, VOXEL_TYPE_LIST));

    voxel_push(executor, voxel_newNumberInt(
        executor->context,
        voxel_findListItemIndex(list, item, equality)
    ));

    voxel_finally:

    voxel_unreferenceThing(executor->context, item);
    voxel_unreferenceThing(executor->context, list);
}

#endif