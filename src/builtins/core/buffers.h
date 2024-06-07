#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_getBufferByte(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);

    if (!bufferThing || bufferThing->type != VOXEL_TYPE_BUFFER || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_Buffer* buffer = (voxel_Buffer*)bufferThing->value;

    if (index < 0) {
        index = buffer->size + index;
    }

    if (index < 0 || index >= buffer->size) {
        voxel_unreferenceThing(executor->context, bufferThing);

        return voxel_pushNull(executor);
    }

    voxel_Byte byte = buffer->value[index];

    voxel_unreferenceThing(executor->context, bufferThing);

    voxel_push(executor, voxel_newByte(executor->context, byte));
}

void voxel_builtins_core_setBufferByte(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);
    voxel_Thing* value = voxel_popByte(executor);

    voxel_push(executor, value);

    if (!bufferThing || bufferThing->type != VOXEL_TYPE_BUFFER || argCount < 3) {
        return voxel_pushNull(executor);
    }

    voxel_Buffer* buffer = (voxel_Buffer*)bufferThing->value;

    if (index < 0) {
        index = buffer->size + index;
    }

    if (index < 0 || index >= buffer->size) {
        voxel_unreferenceThing(executor->context, bufferThing);

        return voxel_pushNull(executor);
    }

    buffer->value[index] = (voxel_Byte)(voxel_IntPtr)value->value;

    voxel_unreferenceThing(executor->context, bufferThing);

    value->referenceCount++;

    voxel_push(executor, value);
}

#endif