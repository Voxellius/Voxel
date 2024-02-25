voxel_Executor* voxel_newExecutor(voxel_Context* context) {
    voxel_Executor* executor = VOXEL_MALLOC(sizeof(voxel_Executor));

    executor->context = context;
    executor->scope = voxel_newScope(context);
    executor->isRunning = VOXEL_TRUE;
    executor->callStack = voxel_newList(context);
    executor->valueStack = voxel_newList(context);
    executor->previousExecutor = context->lastExecutor;
    executor->nextExecutor = VOXEL_NULL;

    if (!context->firstExecutor) {
        context->firstExecutor = executor;
    }

    context->lastExecutor = executor;

    return executor;
}