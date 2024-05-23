#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newObject(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);

    voxel_push(executor, voxel_newObject(executor->context));
}

void voxel_builtins_core_getObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        voxel_unreferenceThing(executor->context, key);
        voxel_unreferenceThing(executor->context, object);

        return voxel_pushNull(executor);
    }

    voxel_Thing* value = objectItem->value;

    value->referenceCount++;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    voxel_push(executor, value);
}

void voxel_builtins_core_setObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_setObjectItem(executor->context, object, key, value);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_removeObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_removeObjectItem(executor->context, object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    voxel_pushNull(executor);
}

void voxel_builtins_core_getObjectItemGetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    if (!objectItem) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->getterFunction) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* getterFunction = descriptor->getterFunction;

    getterFunction->referenceCount++;

    voxel_push(executor, getterFunction);
}

void voxel_builtins_core_setObjectItemGetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        if (VOXEL_IS_ERROR(objectItemResult)) {
            return voxel_pushNull(executor);
        }

        objectItem = objectItemResult.value;
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->getterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->getterFunction);
    }

    descriptor->getterFunction = value;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_getObjectItemSetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    if (!objectItem) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->setterFunction) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* setterFunction = descriptor->setterFunction;

    setterFunction->referenceCount++;

    voxel_push(executor, setterFunction);
}

void voxel_builtins_core_setObjectItemSetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        if (VOXEL_IS_ERROR(objectItemResult)) {
            return voxel_pushNull(executor);
        }

        objectItem = objectItemResult.value;
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->setterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->setterFunction);
    }

    descriptor->setterFunction = value;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_getObjectLength(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getObjectLength(object)));

    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectPrototypes(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT) {
        return;
    }

    voxel_Thing* prototypes = voxel_getObjectPrototypes(executor->context, object);

    prototypes->referenceCount++;

    voxel_push(executor, prototypes);

    voxel_unreferenceThing(executor->context, object);
}

#endif