typedef enum {
    VOXEL_TYPE_NULL,
    VOXEL_TYPE_BOOLEAN,
    VOXEL_TYPE_BYTE,
    VOXEL_TYPE_NUMBER,
    VOXEL_TYPE_BUFFER,
    VOXEL_TYPE_STRING,
    VOXEL_TYPE_NATIVE_FUNCTION,
    VOXEL_TYPE_DEFINED_FUNCTION
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* value;
    voxel_Count referenceCount;
    struct voxel_Thing* previousTrackedThing;
    struct voxel_Thing* nextTrackedThing;
} voxel_Thing;

void voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (thing->referenceCount > 0) {
        thing->referenceCount--;

        return;
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

    switch (thing->type) {
        case VOXEL_TYPE_NULL:
            break;

        default:
            VOXEL_FREE(thing->value);
    }

    VOXEL_FREE(thing);
}

void voxel_removeUnusedThings(voxel_Context* context) {
    voxel_Thing* currentThing = context->firstTrackedThing;

    while (currentThing != NULL) {
        voxel_Thing* nextThing = currentThing->nextTrackedThing;

        if (currentThing->referenceCount == 0) {
            voxel_unreferenceThing(context, currentThing);
        }

        currentThing = nextThing;
    }
}

voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = VOXEL_MALLOC(sizeof(voxel_Thing));

    thing->type = VOXEL_TYPE_NULL;
    thing->value = NULL;
    thing->referenceCount = 1;
    thing->previousTrackedThing = context->lastTrackedThing;
    thing->nextTrackedThing = NULL;

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

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x00 : (void*)0x01;

    return thing;
}

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(long)value;
}