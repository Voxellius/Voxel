#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newBuffer(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int bufferSize = voxel_popNumberInt(executor);

    #ifdef VOXEL_MAX_BUFFER_INIT_SIZE
        if (bufferSize > VOXEL_MAX_BUFFER_INIT_SIZE) {
            bufferSize = VOXEL_MAX_BUFFER_INIT_SIZE;
        }
    #endif

    voxel_push(executor, voxel_newBuffer(executor->context, bufferSize, VOXEL_NULL));
}

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

    if (!bufferThing || bufferThing->type != VOXEL_TYPE_BUFFER || bufferThing->isLocked || argCount < 3) {
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

void voxel_builtins_core_getBufferSize(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* buffer = voxel_pop(executor);

    if (!buffer || buffer->type != VOXEL_TYPE_BUFFER) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getBufferSize(buffer)));

    voxel_unreferenceThing(executor->context, buffer);
}

void voxel_builtins_core_fillBuffer(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int end = voxel_popNumberInt(executor);
    voxel_Int start = voxel_popNumberInt(executor);
    voxel_Thing* byteThing = voxel_popByte(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);

    if (
        !byteThing || !bufferThing ||
        byteThing->type != VOXEL_TYPE_BYTE || bufferThing->type != VOXEL_TYPE_BUFFER ||
        bufferThing->isLocked ||
        argCount < 4
    ) {
        return voxel_push(executor, voxel_newNumberInt(executor->context, -1));
    }

    voxel_Byte byte = (voxel_Byte)(voxel_IntPtr)byteThing->value;
    voxel_Buffer* buffer = (voxel_Buffer*)bufferThing->value;

    if (start < 0) {
        start = buffer->size + start;
    }

    if (end < 0) {
        end = buffer->size + end;
    }

    if (start < 0 || end < 0 || start >= end || start >= buffer->size) {
        voxel_unreferenceThing(executor->context, byteThing);
        voxel_unreferenceThing(executor->context, bufferThing);

        return voxel_push(executor, voxel_newNumberInt(executor->context, -1));
    }

    voxel_Count writeCount = 0;

    while (VOXEL_TRUE) {
        if (writeCount >= VOXEL_MAX_BUFFER_WRITE_BLOCK) {
            // Return next index to start writing from when called again
            break;
        }

        if (start >= end || start >= buffer->size) {
            // Indicate that copying has reached the end
            start = -1;

            break;
        }

        buffer->value[start++] = byte;

        writeCount++;
    }

    voxel_unreferenceThing(executor->context, byteThing);
    voxel_unreferenceThing(executor->context, bufferThing);

    voxel_push(executor, voxel_newNumberInt(executor->context, start));
}

void voxel_builtins_core_copyBufferInto(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int end = voxel_popNumberInt(executor);
    voxel_Int start = voxel_popNumberInt(executor);
    voxel_Int offset = voxel_popNumberInt(executor);
    voxel_Thing* destinationBufferThing = voxel_pop(executor);
    voxel_Thing* sourceBufferThing = voxel_pop(executor);

    if (
        !sourceBufferThing || !destinationBufferThing ||
        sourceBufferThing->type != VOXEL_TYPE_BUFFER ||
        destinationBufferThing->type != VOXEL_TYPE_BUFFER ||
        destinationBufferThing->isLocked ||
        argCount < 5
    ) {
        return voxel_push(executor, voxel_newNumberInt(executor->context, -1));
    }

    voxel_Buffer* sourceBuffer = (voxel_Buffer*)sourceBufferThing->value;
    voxel_Buffer* destinationBuffer = (voxel_Buffer*)destinationBufferThing->value;

    if (start < 0 || end < 0 || offset < 0 || start >= end || start >= sourceBuffer->size || offset >= destinationBuffer->size) {
        voxel_unreferenceThing(executor->context, sourceBufferThing);
        voxel_unreferenceThing(executor->context, destinationBufferThing);

        return voxel_push(executor, voxel_newNumberInt(executor->context, -1));
    }

    voxel_Count copyCount = 0;

    while (VOXEL_TRUE) {
        if (copyCount >= VOXEL_MAX_BUFFER_WRITE_BLOCK) {
            // Return next index to start copying from when called again
            break;
        }

        if (start >= end || start >= sourceBuffer->size || offset >= destinationBuffer->size) {
            // Indicate that copying has reached the end
            start = -1;

            break;
        }

        destinationBuffer->value[offset++] = sourceBuffer->value[start++];

        copyCount++;
    }

    voxel_unreferenceThing(executor->context, sourceBufferThing);
    voxel_unreferenceThing(executor->context, destinationBufferThing);

    voxel_push(executor, voxel_newNumberInt(executor->context, start));
}

#endif