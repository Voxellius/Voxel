#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newBuffer(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Int bufferSize = voxel_popNumberInt(executor);

    #ifdef VOXEL_MAX_BUFFER_INIT_SIZE
        if (bufferSize > VOXEL_MAX_BUFFER_INIT_SIZE) {
            bufferSize = VOXEL_MAX_BUFFER_INIT_SIZE;
        }
    #endif

    voxel_push(executor, voxel_newBuffer(executor->context, bufferSize, VOXEL_NULL));

    voxel_finally:
}

void voxel_builtins_core_bufferToString(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* buffer = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(buffer, VOXEL_TYPE_BUFFER));

    VOXEL_ERRORABLE stringResult = voxel_bufferToString(executor->context, buffer);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(stringResult));

    voxel_push(executor, (voxel_Thing*)stringResult.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, buffer);
}

void voxel_builtins_core_getBufferByte(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(bufferThing, VOXEL_TYPE_BUFFER));

    voxel_Buffer* buffer = (voxel_Buffer*)bufferThing->value;

    if (index < 0) {
        index = buffer->size + index;
    }

    if (index < 0 && index >= buffer->size) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Byte byte = buffer->value[index];

    voxel_push(executor, voxel_newByte(executor->context, byte));

    voxel_finally:

    voxel_unreferenceThing(executor->context, bufferThing);
}

void voxel_builtins_core_setBufferByte(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);
    voxel_Thing* value = voxel_popByte(executor);

    voxel_push(executor, value);

    VOXEL_REQUIRE(VOXEL_ARG(bufferThing, VOXEL_TYPE_BUFFER) && !bufferThing->isLocked);

    voxel_Buffer* buffer = (voxel_Buffer*)bufferThing->value;

    if (index < 0) {
        index = buffer->size + index;
    }

    VOXEL_REQUIRE(index >= 0 && index < buffer->size);

    buffer->value[index] = (voxel_Byte)(voxel_IntPtr)value->value;

    value->referenceCount++;

    voxel_push(executor, value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, bufferThing);
}

void voxel_builtins_core_getBufferSize(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* buffer = voxel_pop(executor);

    VOXEL_REQUIRE(VOXEL_ARG(buffer, VOXEL_TYPE_BUFFER));

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getBufferSize(buffer)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, buffer);
}

void voxel_builtins_core_fillBuffer(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int end = voxel_popNumberInt(executor);
    voxel_Int start = voxel_popNumberInt(executor);
    voxel_Thing* byteThing = voxel_popByte(executor);
    voxel_Thing* bufferThing = voxel_pop(executor);

    if (
        argCount < 4 ||
        !byteThing || !bufferThing ||
        byteThing->type != VOXEL_TYPE_BYTE || bufferThing->type != VOXEL_TYPE_BUFFER ||
        bufferThing->isLocked
    ) {
        voxel_push(executor, voxel_newNumberInt(executor->context, -1));

        goto voxel_finally;
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
        voxel_push(executor, voxel_newNumberInt(executor->context, -1));

        goto voxel_finally;
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

    voxel_push(executor, voxel_newNumberInt(executor->context, start));

    voxel_finally:

    voxel_unreferenceThing(executor->context, byteThing);
    voxel_unreferenceThing(executor->context, bufferThing);
}

void voxel_builtins_core_copyBufferInto(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int end = voxel_popNumberInt(executor);
    voxel_Int start = voxel_popNumberInt(executor);
    voxel_Int offset = voxel_popNumberInt(executor);
    voxel_Thing* destinationBufferThing = voxel_pop(executor);
    voxel_Thing* sourceBufferThing = voxel_pop(executor);

    if (
        argCount < 5 ||
        !sourceBufferThing || !destinationBufferThing ||
        sourceBufferThing->type != VOXEL_TYPE_BUFFER ||
        destinationBufferThing->type != VOXEL_TYPE_BUFFER ||
        destinationBufferThing->isLocked
    ) {
        voxel_push(executor, voxel_newNumberInt(executor->context, -1));

        goto voxel_finally;
    }

    voxel_Buffer* sourceBuffer = (voxel_Buffer*)sourceBufferThing->value;
    voxel_Buffer* destinationBuffer = (voxel_Buffer*)destinationBufferThing->value;

    if (start < 0 || end < 0 || offset < 0 || start >= end || start >= sourceBuffer->size || offset >= destinationBuffer->size) {
        voxel_push(executor, voxel_newNumberInt(executor->context, -1));

        goto voxel_finally;
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

    voxel_push(executor, voxel_newNumberInt(executor->context, start));

    voxel_finally:

    voxel_unreferenceThing(executor->context, sourceBufferThing);
    voxel_unreferenceThing(executor->context, destinationBufferThing);
}

#endif