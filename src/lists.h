voxel_Thing* voxel_newList(voxel_Context* context) {
    voxel_List* list = VOXEL_MALLOC(sizeof(voxel_List));

    list->length = 0;
    list->firstItem = VOXEL_NULL;
    list->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_LIST;
    thing->value = list;

    return thing;
}

VOXEL_ERRORABLE voxel_getListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index) {
    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;

    if (index >= list->length) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    for (voxel_Count i = 0; i < index; i++) {
        if (!currentItem) {
            return VOXEL_OK_RET(VOXEL_NULL);
        }

        currentItem = currentItem->nextItem;
    }

    return VOXEL_OK_RET(currentItem);
}

VOXEL_ERRORABLE voxel_setListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARGUMENT);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARGUMENT);

    if (index == list->length) {
        return voxel_pushOntoList(context, thing, value);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* listItem = listItemResult.value;

    VOXEL_MUST(voxel_unreferenceThing(context, listItem->value));

    listItem->value = value;
    value->referenceCount++;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_removeListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARGUMENT);
    VOXEL_ASSERT(index < list->length, VOXEL_ERROR_INVALID_ARGUMENT);

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* listItem = listItemResult.value;

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = listItem->nextItem;
    }

    if (listItem->nextItem) {
        listItem->nextItem->previousItem = listItem->previousItem;
    }

    if (listItem == list->firstItem) {
        list->firstItem = VOXEL_NULL;
    }

    if (listItem == list->lastItem) {
        list->lastItem = VOXEL_NULL;
    }

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, listItem->value));
    VOXEL_FREE(listItem);

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_pushOntoList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;
    voxel_ListItem* listItem = VOXEL_MALLOC(sizeof(voxel_ListItem));

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = list->lastItem;
    listItem->nextItem = VOXEL_NULL;

    if (list->lastItem) {
        list->lastItem->nextItem = listItem;
    } else {
        list->firstItem = listItem;
    }

    list->length++;
    list->lastItem = listItem;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_popFromList(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_List* list = thing->value;

    if (!list->lastItem) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    voxel_ListItem* listItem = list->lastItem;

    list->lastItem = listItem->previousItem;

    if (listItem == list->firstItem) {
        list->firstItem = VOXEL_NULL;
    }

    voxel_Thing* value = listItem->value;

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, value));
    VOXEL_FREE(listItem);

    return VOXEL_OK_RET(value);
}

VOXEL_ERRORABLE voxel_insertIntoList(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARGUMENT);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARGUMENT);

    if (index == list->length) {
        return voxel_pushOntoList(context, thing, value);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* currentListItem = listItemResult.value;
    voxel_ListItem* listItem = VOXEL_MALLOC(sizeof(voxel_ListItem));

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = currentListItem->previousItem;
    listItem->nextItem = currentListItem;

    if (currentListItem == list->firstItem) {
        list->firstItem = listItem;
    }

    list->length++;

    return VOXEL_OK;
}