voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_String* string = VOXEL_MALLOC(sizeof(voxel_String));

    string->size = size;
    string->value = VOXEL_MALLOC(size);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = string;

    if (size > 0) {
        voxel_copy(data, string->value, size);
    }

    return thing;
}

voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data) {
    voxel_Count size = 0;

    while (data[size] != '\0') {
        size++;
    }

    return voxel_newString(context, size, data);
}

VOXEL_ERRORABLE voxel_destroyString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    VOXEL_FREE(string->value);
    VOXEL_FREE(string);
    VOXEL_FREE(thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    return voxel_compare(aString->value, bString->value, aString->size, bString->size);
}

voxel_Thing* voxel_copyString(voxel_Context* context, voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return voxel_newString(context, string->size, string->value);
}

VOXEL_ERRORABLE voxel_stringToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_String* string = thing->value;
    voxel_Thing* vxONString = voxel_newStringTerminated(context, "\"");

    for (voxel_Count i = 0; i < string->size; i++) {
        switch (string->value[i]) {
            case '"':
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, '"'));
                break;

            case '\0':
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, '0'));
                break;

            case '\n':
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, 'n'));
                break;

            default:
                VOXEL_MUST(voxel_appendByteToString(context, vxONString, string->value[i]));
                break;
        }
    }

    VOXEL_MUST(voxel_appendByteToString(context, vxONString, '"'));

    return VOXEL_OK_RET(vxONString);
}

voxel_Count voxel_getStringSize(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return string->size;
}

void voxel_logString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    for (voxel_Count i = 0; i < string->size; i++) {
        VOXEL_LOG_BYTE(string->value[i]);
    }
}

voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_String* resultString = VOXEL_MALLOC(sizeof(voxel_String));

    resultString->size = aString->size + bString->size;
    resultString->value = VOXEL_MALLOC(resultString->size);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = resultString;

    voxel_Count position = 0;

    for (voxel_Count i = 0; i < aString->size; i++) {
        resultString->value[position++] = aString->value[i];
    }

    for (voxel_Count i = 0; i < bString->size; i++) {
        resultString->value[position++] = bString->value[i];
    }

    return thing;
}

VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ASSERT(!a->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_Count newLength = aString->size + bString->size;

    aString->value = VOXEL_REALLOC(aString->value, newLength);

    for (voxel_Count i = 0; i < bString->size; i++) {
        aString->value[aString->size + i] = bString->value[i];
    }

    aString->size = newLength;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_appendToStringTerminatedBytes(voxel_Context* context, voxel_Thing* a, voxel_Byte* b) {
    voxel_Thing* bThing = voxel_newStringTerminated(context, b);

    VOXEL_MUST(voxel_appendToString(context, a, bThing));
    VOXEL_MUST(voxel_unreferenceThing(context, bThing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_appendByteToString(voxel_Context* context, voxel_Thing* thing, voxel_Byte byte) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;

    string->size++;
    string->value = VOXEL_REALLOC(string->value, string->size);

    string->value[string->size - 1] = byte;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;
    voxel_Byte sourceString[string->size];

    voxel_copy(string->value, sourceString, string->size);

    for (voxel_Count i = 0; i < string->size; i++) {
        string->value[string->size - 1 - i] = sourceString[i];
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_cutStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count size) {
    VOXEL_MUST(voxel_reverseString(context, thing));
    VOXEL_MUST(voxel_cutStringEnd(context, thing, size));
    VOXEL_MUST(voxel_reverseString(context, thing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_cutStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count size) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    VOXEL_ASSERT(size > 0, VOXEL_ERROR_INVALID_ARGUMENT);

    voxel_String* string = thing->value;

    if (string->size < size) {
        return VOXEL_OK;
    }

    string->size = size;
    string->value = VOXEL_REALLOC(string->value, string->size);

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_padStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte) {
    VOXEL_MUST(voxel_reverseString(context, thing));
    VOXEL_MUST(voxel_padStringEnd(context, thing, minSize, byte));
    VOXEL_MUST(voxel_reverseString(context, thing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_padStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;
    voxel_Count padding = minSize - string->size;
    voxel_Count newLength = string->size + padding;

    if (minSize <= 0) {
        return VOXEL_OK;
    }

    string->value = VOXEL_REALLOC(string->value, newLength);

    for (voxel_Count i = 0; i < padding; i++) {
        string->value[string->size + i] = byte;
    }

    string->size = newLength;

    return VOXEL_OK;
}