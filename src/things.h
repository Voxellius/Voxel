voxel_Thing* voxel_newNumberInt(voxel_Context* context, voxel_Int value);

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count length, voxel_Byte* data);
voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data);

void voxel_destroyNull(voxel_Thing* thing);
void voxel_destroyBoolean(voxel_Thing* thing);
void voxel_destroyByte(voxel_Thing* thing);
void voxel_destroyNumber(voxel_Thing* thing);
void voxel_destroyBuffer(voxel_Thing* thing);
void voxel_destroyString(voxel_Thing* thing);
void voxel_destroyObject(voxel_Context* context, voxel_Thing* thing);

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareNumbers(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b);

void voxel_lockObject(voxel_Thing* objectThing);

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_numberToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_bufferToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_stringToString(voxel_Context* context, voxel_Thing* thing);

voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = VOXEL_MALLOC(sizeof(voxel_Thing));

    thing->type = VOXEL_TYPE_NULL;
    thing->value = VOXEL_NULL;
    thing->referenceCount = 1;
    thing->isLocked = VOXEL_FALSE;
    thing->previousTrackedThing = context->lastTrackedThing;
    thing->nextTrackedThing = VOXEL_NULL;

    if (context->lastTrackedThing) {
        context->lastTrackedThing->nextTrackedThing = thing;
    }

    context->lastTrackedThing = thing;

    if (!context->firstTrackedThing) {
        context->firstTrackedThing = thing;
    }

    return thing;
}

voxel_Thing* voxel_newNull(voxel_Context* context) {
    return voxel_newThing(context);
}

void voxel_destroyNull(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_TRUE;
}

VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "null"));
}

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x00 : (void*)0x01;

    return thing;
}

void voxel_destroyBoolean(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, thing->value ? "true" : "false"));
}

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(voxel_IntPtr)value;
}

void voxel_destroyByte(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_byteToNumber(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newNumberInt(context, (voxel_IntPtr)thing->value);
}

VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Byte bytes[1] = {(voxel_IntPtr)thing->value};

    return VOXEL_OK_RET(voxel_newString(context, 1, bytes));
}

VOXEL_ERRORABLE voxel_destroyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: voxel_destroyNull(thing); return VOXEL_OK;
        case VOXEL_TYPE_BOOLEAN: voxel_destroyBoolean(thing); return VOXEL_OK;
        case VOXEL_TYPE_BYTE: voxel_destroyByte(thing); return VOXEL_OK;
        case VOXEL_TYPE_NUMBER: voxel_destroyNumber(thing); return VOXEL_OK;
        case VOXEL_TYPE_BUFFER: voxel_destroyBuffer(thing); return VOXEL_OK;
        case VOXEL_TYPE_STRING: voxel_destroyString(thing); return VOXEL_OK;
        case VOXEL_TYPE_OBJECT: voxel_destroyObject(context, thing); return VOXEL_OK;
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (thing->referenceCount > 0) {
        thing->referenceCount--;

        return VOXEL_OK;
    }

    if (thing == context->firstTrackedThing) {
        context->firstTrackedThing = thing->nextTrackedThing;
    }

    if (thing == context->lastTrackedThing) {
        context->lastTrackedThing = thing->previousTrackedThing;
    }

    if (thing->previousTrackedThing) {
        thing->previousTrackedThing->nextTrackedThing = thing->nextTrackedThing;
    }

    VOXEL_MUST(voxel_destroyThing(context, thing));
}

VOXEL_ERRORABLE voxel_removeUnusedThings(voxel_Context* context) {
    voxel_Thing* currentThing = context->firstTrackedThing;

    while (currentThing != VOXEL_NULL) {
        voxel_Thing* nextThing = currentThing->nextTrackedThing;

        if (currentThing->referenceCount == 0) {
            VOXEL_MUST(voxel_unreferenceThing(context, currentThing));
        }

        currentThing = nextThing;
    }

    return VOXEL_OK;
}

voxel_Bool voxel_compareThingTypes(voxel_Thing* a, voxel_Thing* b) {
    return a->type == b->type;
}

voxel_Bool voxel_compareThings(voxel_Thing* a, voxel_Thing* b) {
    if (!voxel_compareThingTypes(a, b)) {
        return VOXEL_FALSE;
    }

    switch (a->type) {
        case VOXEL_TYPE_NULL: return voxel_compareNulls(a, b);
        case VOXEL_TYPE_BOOLEAN: return voxel_compareBooleans(a, b);
        case VOXEL_TYPE_BYTE: return voxel_compareBytes(a, b);
        case VOXEL_TYPE_NUMBER: return voxel_compareNumbers(a, b);
        case VOXEL_TYPE_BUFFER: return voxel_compareBuffers(a, b);
        case VOXEL_TYPE_STRING: return voxel_compareStrings(a, b);
        // TODO: Compare objects
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning `VOXEL_FALSE` for now");

    return VOXEL_FALSE;
}

void voxel_lockThing(voxel_Thing* thing) {
    if (thing->isLocked) {
        return; // Prevents infinite recursive locking from happening for circular references
    }

    thing->isLocked = VOXEL_TRUE;

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT:
            voxel_lockObject(thing);
            break;
    }
}

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullToString(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToString(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToString(context, thing);
        // TODO: Implement others
    }
}