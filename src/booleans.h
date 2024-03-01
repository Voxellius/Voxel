voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_BOOLEAN);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x00 : (void*)0x01;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyBoolean(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_BOOLEAN);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyBoolean(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newBoolean(context, (voxel_IntPtr)thing->value);
}

VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, thing->value ? "true" : "false"));
}

VOXEL_ERRORABLE voxel_booleanToNumber(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newNumberInt(context, thing->value ? 1 : 0));
}

voxel_Bool voxel_booleanIsTruthy(voxel_Thing* thing) {
    return !!thing->value;
}