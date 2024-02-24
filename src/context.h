voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context));

    context->code = VOXEL_NULL;
    context->firstTrackedThing = VOXEL_NULL;
    context->lastTrackedThing = VOXEL_NULL;

    return context;
}