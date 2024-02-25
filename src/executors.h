voxel_Executor* voxel_newExecutor(voxel_Context* context) {
    voxel_Executor* executor = VOXEL_MALLOC(sizeof(voxel_Executor));

    executor->context = context;
    executor->scope = voxel_newScope(context, context->rootScope);
    executor->isRunning = VOXEL_TRUE;
    executor->callStackSize = VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Count);
    executor->callStack = VOXEL_MALLOC(executor->callStackSize);
    executor->callStack[0] = 0;
    executor->callStackHead = 0;
    executor->valueStack = voxel_newList(context);
    executor->previousExecutor = context->lastExecutor;
    executor->nextExecutor = VOXEL_NULL;

    if (!context->firstExecutor) {
        context->firstExecutor = executor;
    }
    
    if (context->lastExecutor) {
        context->lastExecutor->nextExecutor = executor;
    }

    context->lastExecutor = executor;

    return executor;
}

voxel_Position* voxel_getExecutorPosition(voxel_Executor* executor) {
    return &executor->callStack[executor->callStackHead];
}

VOXEL_ERRORABLE voxel_stepExecutor(voxel_Executor* executor) {
    if (!executor->isRunning) {
        return VOXEL_OK;
    }

    voxel_Position* position = voxel_getExecutorPosition(executor);
    VOXEL_ERRORABLE tokenResult = voxel_nextToken(executor->context, position); VOXEL_MUST(tokenResult);
    voxel_Token* token = tokenResult.value;

    if (!token) {
        executor->isRunning = VOXEL_FALSE;

        return VOXEL_OK;
    }

    switch (token->type) {
        case VOXEL_TOKEN_TYPE_CALL:
            VOXEL_ERRORABLE callFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(callFunctionResult);
            voxel_Thing* callFunction = callFunctionResult.value;

            VOXEL_ASSERT(callFunction, VOXEL_ERROR_MISSING_ARG);
            VOXEL_ASSERT(callFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_CANNOT_CALL_THING);

            voxel_FunctionType functionType = voxel_getFunctionType(executor->context, callFunction);

            if (functionType == VOXEL_FUNCTION_TYPE_BUILTIN) {
                voxel_Count builtinFunctionIndex = (voxel_IntPtr)callFunction->value;

                builtinFunctionIndex *= -1;
                builtinFunctionIndex--;

                VOXEL_ASSERT(
                    builtinFunctionIndex >= 0 && builtinFunctionIndex < executor->context->builtinCount,
                    VOXEL_ERROR_INVALID_BUILTIN
                );

                voxel_Builtin builtin = executor->context->builtins[builtinFunctionIndex];

                (*builtin)(executor);

                break;
            }

            voxel_stepInExecutor(executor, (voxel_Position)(voxel_IntPtr)callFunction->value);

            break;

        case VOXEL_TOKEN_TYPE_RETURN:
            voxel_stepOutExecutor(executor);
            break;

        case VOXEL_TOKEN_TYPE_GET:
            VOXEL_ERRORABLE getKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(getKey);

            VOXEL_ASSERT(getKey.value, VOXEL_ERROR_MISSING_ARG);

            voxel_ObjectItem* scopeItem = voxel_getScopeItem(executor->scope, getKey.value);
            voxel_Thing* scopeValue = scopeItem ? scopeItem->value : voxel_newNull(executor->context);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, scopeValue));

            break;

        case VOXEL_TOKEN_TYPE_SET:
            VOXEL_ERRORABLE setKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(setKey);
            voxel_Thing* setValue = ((voxel_List*)executor->valueStack->value)->lastItem->value;

            VOXEL_ASSERT(setKey.value, VOXEL_ERROR_MISSING_ARG);
            VOXEL_ASSERT(setValue, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST(voxel_setScopeItem(executor->scope, setKey.value, setValue));

            break;

        default:
            // Token contains thing to be pushed onto value stack
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, token->data));
            break;
    }

    return VOXEL_OK;
}

void voxel_stepInExecutor(voxel_Executor* executor, voxel_Position position) {
    voxel_Scope* newScope = voxel_newScope(executor->context, executor->scope);

    executor->scope = newScope;

    executor->callStackHead++;

    voxel_Count neededSize = ((executor->callStackHead / VOXEL_CALL_STACK_BLOCK_LENGTH) + 1) * sizeof(voxel_Count);

    if (executor->callStackSize < neededSize) {
        executor->callStackSize = neededSize;
        executor->callStack = VOXEL_REALLOC(executor->callStack, neededSize);
    }

    executor->callStack[executor->callStackHead] = position;
}

void voxel_stepOutExecutor(voxel_Executor* executor) {
    if (executor->callStackHead == 0) {
        executor->isRunning = VOXEL_FALSE;

        return;
    }

    voxel_Scope* parentScope = executor->scope->parentScope;

    if (parentScope) {
        voxel_destroyScope(executor->scope);

        executor->scope = parentScope;
    }

    executor->callStackHead--;
}