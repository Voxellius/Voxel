typedef struct voxel_Object {
    voxel_Count length;
    struct voxel_ObjectItem* firstItem;
    struct voxel_ObjectItem* lastItem;
} voxel_Object;

typedef struct voxel_ObjectItem {
    voxel_Thing* key;
    voxel_Thing* value;
    struct voxel_ObjectItem* nextItem;
} voxel_ObjectItem;

voxel_Thing* voxel_newObject(voxel_Context* context) {
    voxel_Object* object = VOXEL_MALLOC(sizeof(voxel_Object));

    object->length = 0;
    object->firstItem = VOXEL_NULL;
    object->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_OBJECT;

    return thing;
}

voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* objectThing, voxel_Thing* key) {
    voxel_Object* object = objectThing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (VOXEL_TRUE) {
        if (currentItem == VOXEL_NULL) {
            return VOXEL_NULL;
        }

        if (voxel_compareThings(currentItem->key, key)) {
            return currentItem;
        }

        currentItem = currentItem->nextItem;
    }

    return VOXEL_NULL;
}

void voxel_setObjectItem(voxel_Context* context, voxel_Thing* objectThing, voxel_Thing* key, voxel_Thing* value) {
    voxel_Object* object = objectThing->value;
    voxel_ObjectItem* objectItem = voxel_getObjectItem(objectThing, key);

    if (objectItem) {
        voxel_unreferenceThing(context, objectItem->value);

        objectItem->value = value;
        value->referenceCount++;

        return;
    }

    objectItem = VOXEL_MALLOC(sizeof(voxel_ObjectItem));

    objectItem->key = key;
    key->referenceCount++;

    objectItem->value = value;
    value->referenceCount++;

    objectItem->nextItem = VOXEL_NULL;

    if (object->firstItem == VOXEL_NULL) {
        object->firstItem = objectItem;
    }

    if (object->lastItem != VOXEL_NULL) {
        object->lastItem->nextItem = objectItem;
    }

    object->length++;
    object->lastItem = objectItem;
}