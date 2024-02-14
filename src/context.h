typedef struct voxel_Context {
    char* code;
    voxel_Count codeLength;
    struct voxel_Token* tokens;
    voxel_Count tokenCount;
    voxel_Count currentPosition;
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