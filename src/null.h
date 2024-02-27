voxel_Thing* voxel_newNull(voxel_Context* context) {
    return voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NULL);
}

VOXEL_ERRORABLE voxel_destroyNull(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_NULL);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_TRUE;
}

voxel_Thing* voxel_copyNull(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NULL);
}

VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "null"));
}