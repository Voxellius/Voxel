voxel_Thing* voxel_newNull(voxel_Context* context) {
    return voxel_newThing(context);
}

void voxel_destroyNull(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_TRUE;
}

voxel_Thing* voxel_copyNull(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newThing(context);
}

VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "null"));
}