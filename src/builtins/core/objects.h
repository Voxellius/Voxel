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

    value->referenceCount += 2;

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

void voxel_builtins_core_getObjectLength(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT) {
        return;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getObjectLength(object)));

    voxel_unreferenceThing(executor->context, object);
}

#endif