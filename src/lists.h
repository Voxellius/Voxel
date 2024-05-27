voxel_Thing* voxel_newList(voxel_Context* context) {
    voxel_List* list = (voxel_List*)VOXEL_MALLOC(sizeof(voxel_List)); VOXEL_TAG_MALLOC(voxel_List);

    list->length = 0;
    list->firstItem = VOXEL_NULL;
    list->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_LIST);

    thing->type = VOXEL_TYPE_LIST;
    thing->value = list;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyList(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_LIST);

    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;
    voxel_ListItem* nextItem;

    while (currentItem) {
        VOXEL_MUST(voxel_unreferenceThing(context, currentItem->value));

        nextItem = currentItem->nextItem;

        VOXEL_FREE(currentItem); VOXEL_TAG_FREE(voxel_ListItem);

        currentItem = nextItem;
    }

    VOXEL_FREE(list); VOXEL_TAG_FREE(voxel_List);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareLists(voxel_Thing* a, voxel_Thing* b) {
    voxel_List* aList = a->value;
    voxel_List* bList = b->value;

    if (aList->length != bList->length) {
        return VOXEL_FALSE;
    }

    voxel_ListItem* aCurrentItem = aList->firstItem;
    voxel_ListItem* bCurrentItem = bList->firstItem;

    while (VOXEL_TRUE) {
        if (!aCurrentItem && !bCurrentItem) {
            break;
        }

        if (!voxel_compareThings(aCurrentItem->value, bCurrentItem->value)) {
            return VOXEL_FALSE;
        }

        if (!aCurrentItem) {
            return VOXEL_FALSE;
        }

        if (!bCurrentItem) {
            return VOXEL_FALSE;
        }

        aCurrentItem = aCurrentItem->nextItem;
        bCurrentItem = bCurrentItem->nextItem;
    }

    return VOXEL_TRUE;
}

void voxel_lockList(voxel_Thing* thing) {
    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        voxel_lockThing(currentItem->value);

        currentItem = currentItem->nextItem;
    }
}

voxel_Thing* voxel_copyList(voxel_Context* context, voxel_Thing* thing) {
    // Shallow copy only; deep copying will be handled in the Voxel standard library

    voxel_List* list = thing->value;
    voxel_Thing* newList = voxel_newList(context);
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        voxel_pushOntoList(context, newList, currentItem->value);

        currentItem = currentItem->nextItem;
    }

    return newList;
}

VOXEL_ERRORABLE voxel_listToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* delimeter = voxel_newStringTerminated(context, ",");
    VOXEL_ERRORABLE string = voxel_joinList(context, thing, delimeter);

    VOXEL_MUST(voxel_unreferenceThing(context, delimeter));

    return string;
}

VOXEL_ERRORABLE voxel_listToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_List* list = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "[");
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        VOXEL_ERRORABLE itemString = voxel_thingToVxon(context, currentItem->value); VOXEL_MUST(itemString);

        VOXEL_MUST(voxel_appendToString(context, string, itemString.value));
        VOXEL_MUST(voxel_unreferenceThing(context, itemString.value));

        currentItem = currentItem->nextItem;

        if (currentItem) {
            VOXEL_MUST(voxel_appendByteToString(context, string, ','));
        }
    }

    VOXEL_MUST(voxel_appendByteToString(context, string, ']'));

    return VOXEL_OK_RET(string);
}

voxel_Bool voxel_listIsTruthy(voxel_Thing* thing) {
    voxel_List* list = thing->value;

    return list->length != 0;
}

VOXEL_ERRORABLE voxel_getListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index) {
    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;

    if (index >= list->length) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    if (index == list->length - 1) {
        return VOXEL_OK_RET(list->lastItem);
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

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARG);

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

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index < list->length, VOXEL_ERROR_INVALID_ARG);

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* listItem = listItemResult.value;

    if (listItem == list->firstItem) {
        list->firstItem = listItem->nextItem;
    }

    if (listItem == list->lastItem) {
        list->lastItem = listItem->previousItem;
    }

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = listItem->nextItem;
    }

    if (listItem->nextItem) {
        listItem->nextItem->previousItem = listItem->previousItem;
    }

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, listItem->value));
    VOXEL_FREE(listItem); VOXEL_TAG_FREE(voxel_ListItem);

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_pushOntoList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;
    voxel_ListItem* listItem = (voxel_ListItem*)VOXEL_MALLOC(sizeof(voxel_ListItem)); VOXEL_TAG_MALLOC(voxel_ListItem);

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = list->lastItem;
    listItem->nextItem = VOXEL_NULL;

    if (list->lastItem) {
        list->lastItem->nextItem = listItem;
    }

    if (!list->firstItem) {
        list->firstItem = listItem;
    }

    list->length++;
    list->lastItem = listItem;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_popFromList(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_List* list = thing->value;
    voxel_ListItem* listItem = list->lastItem;

    if (!listItem) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = VOXEL_NULL;
    }

    list->lastItem = listItem->previousItem;

    if (listItem == list->firstItem) {
        list->firstItem = VOXEL_NULL;
    }

    voxel_Thing* value = listItem->value;

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, value));
    VOXEL_FREE(listItem); VOXEL_TAG_FREE(voxel_ListItem);

    return VOXEL_OK_RET(value);
}

VOXEL_ERRORABLE voxel_insertIntoList(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARG);

    if (index == list->length) {
        return voxel_pushOntoList(context, thing, value);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* currentListItem = listItemResult.value;
    voxel_ListItem* listItem = (voxel_ListItem*)VOXEL_MALLOC(sizeof(voxel_ListItem)); VOXEL_TAG_MALLOC(voxel_ListItem);

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = currentListItem->previousItem;
    listItem->nextItem = currentListItem;
    currentListItem->previousItem = listItem;

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = listItem;
    }

    if (currentListItem == list->firstItem) {
        list->firstItem = listItem;
    }

    list->length++;

    return VOXEL_OK;
}

voxel_Count voxel_getListLength(voxel_Thing* thing) {
    voxel_List* list = thing->value;

    return list->length;
}

VOXEL_ERRORABLE voxel_joinList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* delimeter) {
    voxel_List* list = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "");
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        VOXEL_ERRORABLE itemString = voxel_thingToString(context, currentItem->value); VOXEL_MUST(itemString);

        VOXEL_MUST(voxel_appendToString(context, string, itemString.value));
        VOXEL_MUST(voxel_unreferenceThing(context, itemString.value));

        currentItem = currentItem->nextItem;

        if (delimeter && currentItem) {
            VOXEL_MUST(voxel_appendToString(context, string, delimeter));
        }
    }

    return VOXEL_OK_RET(string);
}