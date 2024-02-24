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