voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context)); VOXEL_TAG_MALLOC(voxel_Context);

    context->code = VOXEL_NULL;
    context->builtins = VOXEL_MALLOC(0); VOXEL_TAG_MALLOC_SIZE("voxel_Context->builtins", 0);
    context->builtinCount = 0;
    context->firstTrackedThing = VOXEL_NULL;
    context->lastTrackedThing = VOXEL_NULL;
    context->firstExecutor = VOXEL_NULL;
    context->lastExecutor = VOXEL_NULL;
    context->rootScope = voxel_newScope(context, VOXEL_NULL);

    voxel_newExecutor(context);

    return context;
}

VOXEL_ERRORABLE voxel_stepContext(voxel_Context* context) {
    voxel_Executor* currentExecutor = context->firstExecutor;

    while (currentExecutor) {
        VOXEL_MUST(voxel_stepExecutor(currentExecutor));

        currentExecutor = currentExecutor->nextExecutor;
    }

    voxel_removeUnusedThings(context);

    return VOXEL_OK;
}

voxel_Bool voxel_anyContextsRunning(voxel_Context* context) {
    voxel_Executor* currentExecutor = context->firstExecutor;

    while (currentExecutor) {
        if (!currentExecutor->isRunning) {
            return VOXEL_FALSE;
        }

        currentExecutor = currentExecutor->nextExecutor;
    }

    return VOXEL_TRUE;
}

VOXEL_ERRORABLE voxel_defineBuiltin(voxel_Context* context, voxel_Byte* name, voxel_Builtin builtin) {
    voxel_Thing* key = voxel_newStringTerminated(context, name);

    context->builtinCount++;

    context->builtins = VOXEL_REALLOC(context->builtins, context->builtinCount); VOXEL_TAG_REALLOC("voxel_Context->builtins", context->builtinCount - 1, context->builtinCount);
    context->builtins[context->builtinCount - 1] = builtin;

    voxel_Thing* function = voxel_newFunctionBuiltin(context, context->builtinCount - 1);

    VOXEL_MUST(voxel_setScopeItem(context->rootScope, key, function));

    return VOXEL_OK;
}