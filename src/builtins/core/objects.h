#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newObject(voxel_Executor* executor) {
    VOXEL_ARGC(0);

    voxel_push(executor, voxel_newObject(executor->context));

    voxel_finally:
}

void voxel_builtins_core_getObjectItem(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT)
    );

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Thing* value = objectItem->value;

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_setObjectItem(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || object->isLocked) {
        goto voxel_finally;
    }

    voxel_setObjectItem(executor->context, object, key, value);

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_removeObjectItem(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT) && !object->isLocked
    );

    voxel_removeObjectItem(executor->context, object, key);

    voxel_pushNull(executor);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectItemGetter(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT)
    );

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->getterFunction) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Thing* getterFunction = descriptor->getterFunction;

    getterFunction->referenceCount++;

    voxel_push(executor, getterFunction);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_setObjectItemGetter(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT) && !object->isLocked &&
        value
    );

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        VOXEL_REQUIRE(!VOXEL_IS_ERROR(objectItemResult));

        objectItem = (voxel_ObjectItem*)objectItemResult.value;

        voxel_unreferenceThing(executor->context, objectItem->value);
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->getterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->getterFunction);
    }

    descriptor->getterFunction = value;

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectItemSetter(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT)
    );

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->setterFunction) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Thing* setterFunction = descriptor->setterFunction;

    setterFunction->referenceCount++;

    voxel_push(executor, setterFunction);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_setObjectItemSetter(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    VOXEL_REQUIRE(
        key &&
        VOXEL_ARG(object, VOXEL_TYPE_OBJECT) && !object->isLocked &&
        value
    );

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        VOXEL_REQUIRE(!VOXEL_IS_ERROR(objectItemResult));

        objectItem = (voxel_ObjectItem*)objectItemResult.value;

        voxel_unreferenceThing(executor->context, objectItem->value);
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->setterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->setterFunction);
    }

    descriptor->setterFunction = value;

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectLength(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(object, VOXEL_TYPE_OBJECT));

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getObjectLength(object)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectKeys(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(object, VOXEL_TYPE_OBJECT));

    VOXEL_ERRORABLE keysResult = voxel_getObjectKeys(executor->context, object, VOXEL_MAX_PROTOTYPE_TRAVERSE_DEPTH);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(keysResult))

    voxel_Thing* keys = (voxel_Thing*)keysResult.value;

    voxel_push(executor, keys);

    voxel_finally:

    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectPrototypes(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* object = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(object, VOXEL_TYPE_OBJECT));

    voxel_Thing* prototypes = voxel_getObjectPrototypes(executor->context, object);

    prototypes->referenceCount++;

    voxel_push(executor, prototypes);

    voxel_finally:

    voxel_unreferenceThing(executor->context, object);
}

#endif