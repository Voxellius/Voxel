typedef struct voxel_Context {
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
} voxel_Context;

voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context));

    context->firstTrackedThing = NULL;
    context->lastTrackedThing = NULL;

    return context;
}