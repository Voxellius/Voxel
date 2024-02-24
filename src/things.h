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
    if (a == b) {
        return VOXEL_TRUE;
    }

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
        case VOXEL_TYPE_OBJECT: return voxel_compareObjects(a, b);
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

voxel_Thing* voxel_copyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_copyNull(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_copyBoolean(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_copyByte(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_copyNumber(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_copyBuffer(context, thing);
        case VOXEL_TYPE_STRING: return voxel_copyString(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_copyObject(context, thing);
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning null thing for now");

    return voxel_newNull(context);
}

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullToString(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToString(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToString(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_numberToString(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferToString(context, thing);
        case VOXEL_TYPE_STRING: return VOXEL_OK_RET(voxel_copyString(context, thing));
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_thingToVxon(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_BYTE: return voxel_byteToVxon(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferToVxon(context, thing);
        case VOXEL_TYPE_STRING: return voxel_stringToVxon(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
    }

    return voxel_thingToString(context, thing);
}