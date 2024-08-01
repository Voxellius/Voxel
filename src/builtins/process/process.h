#ifdef VOXEL_BUILTINS_PROCESS

void voxel_builtins_core_getProcessArgs(voxel_Executor* executor) {
    VOXEL_ARGC(0);

    voxel_Thing* argsList = executor->context->argsList;

    argsList->referenceCount++;

    voxel_push(executor, argsList);

    voxel_finally:
}

void voxel_builtins_process(voxel_Context* context) {
    voxel_defineBuiltin(context, ".process_args", &voxel_builtins_core_getProcessArgs);
}

#else

void voxel_builtins_process(voxel_Context* context) {}

#endif