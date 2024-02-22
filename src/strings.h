typedef struct voxel_String {
    voxel_Count length;
    voxel_Byte* value;
} voxel_String;

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count length, voxel_Byte* data) {
    voxel_String* string = VOXEL_MALLOC(sizeof(voxel_String));

    string->length = length;
    string->value = VOXEL_MALLOC(length);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = string;

    if (length > 0) {
        voxel_copy(data, string->value, length);
    }

    return thing;
}

voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data) {
    voxel_Count length = 0;

    while (data[length] != '\0') {
        length++;
    }

    return voxel_newString(context, length, data);
}

void voxel_destroyString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    VOXEL_FREE(string->value);
    VOXEL_FREE(string);
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    return voxel_compare(aString->value, bString->value, aString->length, bString->length);
}

void voxel_logString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    for (voxel_Count i = 0; i < string->length; i++) {
        VOXEL_LOG_BYTE(string->value[i]);
    }
}

voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_String* resultString = VOXEL_MALLOC(sizeof(voxel_String));

    resultString->length = aString->length + bString->length;
    resultString->value = VOXEL_MALLOC(resultString->length);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = resultString;

    voxel_Count position = 0;

    for (voxel_Count i = 0; i < aString->length; i++) {
        resultString->value[position++] = aString->value[i];
    }

    for (voxel_Count i = 0; i < bString->length; i++) {
        resultString->value[position++] = bString->value[i];
    }

    return thing;
}

VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ASSERT(!a->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_Count newLength = aString->length + bString->length;

    aString->value = VOXEL_REALLOC(aString->value, newLength);

    for (voxel_Count i = 0; i < bString->length; i++) {
        aString->value[aString->length + i] = bString->value[i];
    }

    aString->length = newLength;

    return VOXEL_OK;
}