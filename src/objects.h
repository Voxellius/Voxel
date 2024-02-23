voxel_Thing* voxel_newObject(voxel_Context* context) {
    voxel_Object* object = VOXEL_MALLOC(sizeof(voxel_Object));

    object->length = 0;
    object->firstItem = VOXEL_NULL;
    object->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_OBJECT;
    thing->value = object;

    return thing;
}

voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* thing, voxel_Thing* key) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            return VOXEL_NULL;
        }

        if (voxel_compareThings(currentItem->key, key)) {
            return currentItem;
        }

        currentItem = currentItem->nextItem;
    }
}

VOXEL_ERRORABLE voxel_setObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_Object* object = thing->value;
    voxel_ObjectItem* objectItem = voxel_getObjectItem(thing, key);

    if (objectItem) {
        voxel_unreferenceThing(context, objectItem->value);

        objectItem->value = value;
        value->referenceCount++;

        return VOXEL_OK;
    }

    voxel_lockThing(key);

    objectItem = VOXEL_MALLOC(sizeof(voxel_ObjectItem));

    objectItem->key = key;
    key->referenceCount++;

    objectItem->value = value;
    value->referenceCount++;

    objectItem->nextItem = VOXEL_NULL;

    if (!object->firstItem) {
        object->firstItem = objectItem;
    }

    if (object->lastItem) {
        object->lastItem->nextItem = objectItem;
    }

    object->length++;
    object->lastItem = objectItem;

    return VOXEL_OK;
}

VOXEL_ERRORABLE removeObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* previousItem = VOXEL_NULL;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            VOXEL_THROW(VOXEL_ERROR_THING_LOCKED);
        }

        if (voxel_compareThings(currentItem->key, key)) {
            if (currentItem == object->firstItem) {
                object->firstItem = currentItem->nextItem;
            } else {
                previousItem->nextItem = currentItem->nextItem;
            }

            if (currentItem == object->lastItem) {
                object->lastItem = previousItem;
            }

            voxel_unreferenceThing(context, currentItem->key);
            voxel_unreferenceThing(context, currentItem->value);

            VOXEL_FREE(currentItem);

            object->length--;

            return VOXEL_OK;
        }

        previousItem = currentItem;
        currentItem = currentItem->nextItem;
    }
}

void voxel_destroyObject(voxel_Context* context, voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* nextItem;

    while (currentItem) {
        voxel_unreferenceThing(context, currentItem->key);
        voxel_unreferenceThing(context, currentItem->value);

        nextItem = currentItem->nextItem;

        VOXEL_FREE(currentItem);

        currentItem = nextItem;
    }

    VOXEL_FREE(object);
    VOXEL_FREE(thing);
}

void voxel_lockObject(voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (currentItem) {
        voxel_lockThing(currentItem->value);

        currentItem = currentItem->nextItem;
    }
}