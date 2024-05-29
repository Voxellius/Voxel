voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = (voxel_Thing*)VOXEL_MALLOC(sizeof(voxel_Thing)); VOXEL_TAG_MALLOC(voxel_Thing);

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
        case VOXEL_TYPE_NULL: return voxel_destroyNull(thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_destroyBoolean(thing);
        case VOXEL_TYPE_BYTE: return voxel_destroyByte(thing);
        case VOXEL_TYPE_FUNCTION: return voxel_destroyFunction(thing);
        case VOXEL_TYPE_CLOSURE: return voxel_destroyClosure(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_destroyNumber(thing);
        case VOXEL_TYPE_BUFFER: return voxel_destroyBuffer(thing);
        case VOXEL_TYPE_STRING: return voxel_destroyString(thing);
        case VOXEL_TYPE_OBJECT: return voxel_destroyObject(context, thing);
        case VOXEL_TYPE_LIST: return voxel_destroyList(context, thing);
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (!thing) {
        return VOXEL_OK;
    }

    if (thing->referenceCount > 0) {
        thing->referenceCount--;
    }

    if (thing->referenceCount > 0) {
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

    if (thing->nextTrackedThing) {
        thing->nextTrackedThing->previousTrackedThing = thing->previousTrackedThing;
    }

    VOXEL_MUST(voxel_destroyThing(context, thing));

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
        case VOXEL_TYPE_FUNCTION: return voxel_compareFunctions(a, b);
        case VOXEL_TYPE_CLOSURE: return voxel_compareClosures(a, b);
        case VOXEL_TYPE_NUMBER: return voxel_compareNumbers(a, b);
        case VOXEL_TYPE_BUFFER: return voxel_compareBuffers(a, b);
        case VOXEL_TYPE_STRING: return voxel_compareStrings(a, b);
        case VOXEL_TYPE_OBJECT: return voxel_compareObjects(a, b);
        case VOXEL_TYPE_LIST: return voxel_compareLists(a, b);
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning `VOXEL_FALSE` for now\n");

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

        case VOXEL_TYPE_LIST:
            voxel_lockList(thing);
            break;
    }
}

voxel_Thing* voxel_copyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_copyNull(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_copyBoolean(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_copyByte(context, thing);
        case VOXEL_TYPE_FUNCTION: return voxel_copyFunction(context, thing);
        case VOXEL_TYPE_CLOSURE: return voxel_copyClosure(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_copyNumber(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_copyBuffer(context, thing);
        case VOXEL_TYPE_STRING: return voxel_copyString(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_copyObject(context, thing);
        case VOXEL_TYPE_LIST: return voxel_copyList(context, thing);
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning null thing for now\n");

    return voxel_newNull(context);
}

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullToString(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToString(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToString(context, thing);
        case VOXEL_TYPE_FUNCTION: return voxel_functionToString(context, thing);
        case VOXEL_TYPE_CLOSURE: return voxel_closureToString(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_numberToString(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferToString(context, thing);
        case VOXEL_TYPE_STRING: return VOXEL_OK_RET(voxel_copyString(context, thing));
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
        case VOXEL_TYPE_LIST: return voxel_listToString(context, thing);
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_thingToVxon(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_BYTE: return voxel_byteToVxon(context, thing);

        case VOXEL_TYPE_FUNCTION:
            #ifdef VOXEL_DEBUG_EXECUTORS
                break;
            #else
                VOXEL_THROW(VOXEL_ERROR_CANNOT_CONVERT_THING);
            #endif

        case VOXEL_TYPE_BUFFER: return voxel_bufferToVxon(context, thing);
        case VOXEL_TYPE_STRING: return voxel_stringToVxon(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
        case VOXEL_TYPE_LIST: return voxel_listToVxon(context, thing);
    }

    return voxel_thingToString(context, thing);
}

VOXEL_ERRORABLE voxel_thingToNumber(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return VOXEL_OK_RET(voxel_newNumberInt(context, 0));
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToNumber(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToNumber(context, thing);
        case VOXEL_TYPE_FUNCTION: case VOXEL_TYPE_CLOSURE: return VOXEL_OK_RET(voxel_newNumberInt(context, 1));
        case VOXEL_TYPE_NUMBER: return VOXEL_OK_RET(voxel_copyNumber(context, thing));
        case VOXEL_TYPE_BUFFER: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getBufferSize(thing)));
        case VOXEL_TYPE_STRING: return voxel_stringToNumber(context, thing);
        case VOXEL_TYPE_OBJECT: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getObjectLength(thing)));
        case VOXEL_TYPE_LIST: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getListLength(thing)));
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_thingToByte(voxel_Context* context, voxel_Thing* thing) {
    if (thing->type == VOXEL_TYPE_BYTE) {
        return VOXEL_OK_RET(voxel_copyByte(context, thing));
    }

    VOXEL_ERRORABLE number = voxel_thingToNumber(context, thing); VOXEL_MUST(number);

    return voxel_numberToByte(context, (voxel_Thing*)number.value);
}

voxel_Bool voxel_thingIsTruthy(voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullIsTruthy(thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanIsTruthy(thing);
        case VOXEL_TYPE_BYTE: return voxel_byteIsTruthy(thing);
        case VOXEL_TYPE_FUNCTION: return voxel_functionIsTruthy(thing);
        case VOXEL_TYPE_CLOSURE: return voxel_closureIsTruthy(thing);
        case VOXEL_TYPE_NUMBER: return voxel_numberIsTruthy(thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferIsTruthy(thing);
        case VOXEL_TYPE_STRING: return voxel_stringIsTruthy(thing);
        case VOXEL_TYPE_OBJECT: return voxel_objectIsTruthy(thing);
        case VOXEL_TYPE_LIST: return voxel_listIsTruthy(thing);
    }

    VOXEL_DEBUG_LOG("Thing truthiness not implemented; returning null thing for now\n");

    return VOXEL_FALSE;
}

VOXEL_ERRORABLE voxel_logThing(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ERRORABLE string = voxel_thingToString(context, thing); VOXEL_MUST(string);

    voxel_logString((voxel_Thing*)string.value);

    return voxel_unreferenceThing(context, (voxel_Thing*)string.value);
}