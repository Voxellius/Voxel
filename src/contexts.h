voxel_Context* voxel_newContext() {
    voxel_Context* context = (voxel_Context*)VOXEL_MALLOC(sizeof(voxel_Context)); VOXEL_TAG_MALLOC(voxel_Context);

    context->isInitialised = VOXEL_FALSE;
    context->code = VOXEL_NULL;
    context->codeLength = 0;
    context->builtins = (voxel_Builtin*)VOXEL_MALLOC(0); VOXEL_TAG_MALLOC_SIZE("voxel_Context->builtins", 0);
    context->builtinCount = 0;
    context->tokenisationState = VOXEL_STATE_NONE;
    context->firstTrackedThing = VOXEL_NULL;
    context->lastTrackedThing = VOXEL_NULL;
    context->firstExecutor = VOXEL_NULL;
    context->lastExecutor = VOXEL_NULL;
    context->globalScope = voxel_newScope(context, VOXEL_NULL);

    voxel_newExecutor(context);

    voxel_builtins_core(context);

    return context;
}

VOXEL_ERRORABLE voxel_initContext(voxel_Context* context) {
    if (context->isInitialised) {
        return VOXEL_OK;
    }

    #ifdef VOXEL_MAGIC
        if (context->codeLength < VOXEL_MAGIC_SIZE) {
            VOXEL_THROW(VOXEL_ERROR_INVALID_MAGIC);
        }

        static voxel_Byte* magic = (voxel_Byte[VOXEL_MAGIC_SIZE]) {VOXEL_MAGIC};

        for (voxel_Count i = 0; i < VOXEL_MAGIC_SIZE; i++) {
            if (context->code[i] != magic[i]) {
                VOXEL_THROW(VOXEL_ERROR_INVALID_MAGIC);
            }
        }
    #endif

    context->isInitialised = VOXEL_TRUE;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_stepContext(voxel_Context* context) {
    VOXEL_ASSERT(context->isInitialised, VOXEL_ERROR_NOT_INITIALISED);

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

    voxel_Count newSize = context->builtinCount * sizeof(voxel_Builtin);

    context->builtins = (voxel_Builtin*)VOXEL_REALLOC(context->builtins, newSize); VOXEL_TAG_REALLOC("voxel_Context->builtins", newSize - sizeof(voxel_Builtin), newSize);
    context->builtins[context->builtinCount - 1] = builtin;

    voxel_Thing* function = voxel_newFunctionBuiltin(context, context->builtinCount - 1);

    VOXEL_MUST(voxel_setScopeItem(context->globalScope, key, function));

    return VOXEL_OK;
}