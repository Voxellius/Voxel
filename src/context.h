typedef struct voxel_Context {
    char* code;
    struct voxel_Token* tokens;
    VOXEL_COUNT tokenCount;
    VOXEL_COUNT currentPosition;
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
} voxel_Context;

voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context));

    context->code = NULL;
    context->tokens = NULL;
    context->tokenCount = 0;
    context->currentPosition = 0;
    context->firstTrackedThing = NULL;
    context->lastTrackedThing = NULL;

    return context;
}