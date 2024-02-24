voxel_Thing* voxel_newBuffer(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_Buffer* buffer = VOXEL_MALLOC(sizeof(voxel_Buffer));

    buffer->size = size;
    buffer->value = VOXEL_MALLOC(size);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BUFFER;
    thing->value = buffer;

    if (data != VOXEL_NULL) {
        voxel_copy(data, buffer->value, size);
    } else {
        for (voxel_Count i = 0; i < size; i++) {
            buffer->value[i] = 0;
        }
    }

    return thing;
}

VOXEL_ERRORABLE voxel_destroyBuffer(voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    VOXEL_FREE(buffer->value);
    VOXEL_FREE(buffer);
    VOXEL_FREE(thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b) {
    voxel_Buffer* aBuffer = a->value;
    voxel_Buffer* bBuffer = b->value;

    return voxel_compare(aBuffer->value, bBuffer->value, aBuffer->size, bBuffer->size);
}

voxel_Thing* voxel_copyBuffer(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;
    
    return voxel_newBuffer(context, buffer->size, buffer->value);
}

VOXEL_ERRORABLE voxel_bufferToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    return VOXEL_OK_RET(voxel_newString(context, buffer->size, buffer->value));
}

VOXEL_ERRORABLE voxel_bufferToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "buffer([");
    voxel_Thing* hexPrefix = voxel_newStringTerminated(context, "0x");
    voxel_Thing* suffix = voxel_newStringTerminated(context, "])");

    for (voxel_Count i = 0; i < buffer->size; i++) {
        voxel_Thing* number = voxel_newNumberInt(context, buffer->value[i]);
        VOXEL_ERRORABLE hexString = voxel_numberToBaseString(context, number, 16, 2); VOXEL_MUST(hexString);

        VOXEL_MUST(voxel_appendToString(context, string, hexPrefix));
        VOXEL_MUST(voxel_appendToString(context, string, hexString.value));

        if (i < buffer->size - 1) {
            VOXEL_MUST(voxel_appendByteToString(context, string, ','));
        }

        VOXEL_MUST(voxel_unreferenceThing(context, number));
        VOXEL_MUST(voxel_unreferenceThing(context, hexString.value));
    }

    VOXEL_MUST(voxel_appendToString(context, string, suffix));

    VOXEL_MUST(voxel_unreferenceThing(context, hexPrefix));
    VOXEL_MUST(voxel_unreferenceThing(context, suffix));

    return VOXEL_OK_RET(string);
}