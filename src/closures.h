voxel_Thing* voxel_newClosure(voxel_Context* context, voxel_Position positionReference, voxel_Thing* environment) {
    voxel_Closure* closure = (voxel_Closure*)VOXEL_MALLOC(sizeof(voxel_Closure)); VOXEL_TAG_MALLOC(voxel_Closure);

    closure->position = positionReference;
    closure->environment = environment;

    environment->referenceCount++;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_CLOSURE);

    thing->type = VOXEL_TYPE_CLOSURE;
    thing->value = closure;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyClosure(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_CLOSURE);

    voxel_Closure* closure = (voxel_Closure*)thing->value;

    VOXEL_MUST(voxel_unreferenceThing(context, closure->environment));

    VOXEL_FREE(closure); VOXEL_TAG_FREE(voxel_Closure);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareClosures(voxel_Thing* a, voxel_Thing* b) {
    voxel_Closure* aClosure = (voxel_Closure*)a->value;
    voxel_Closure* bClosure = (voxel_Closure*)b->value;

    if (aClosure->position != bClosure->position) {
        return VOXEL_FALSE;
    }

    return voxel_compareObjects(aClosure->environment, bClosure->environment);
}

voxel_Thing* voxel_copyClosure(voxel_Context* context, voxel_Thing* thing) {
    voxel_Closure* closure = (voxel_Closure*)thing->value;

    return voxel_newClosure(context, closure->position, closure->environment);
}

VOXEL_ERRORABLE voxel_closureToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "(closure function)"));
}

voxel_Bool voxel_closureIsTruthy(voxel_Thing* thing) {
    return VOXEL_TRUE;
}