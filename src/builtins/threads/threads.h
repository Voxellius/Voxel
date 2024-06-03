#ifdef VOXEL_BUILTINS_THREADS

void voxel_builtins_threads_newThread(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* callArgs = voxel_pop(executor);
    voxel_Thing* callFunction = voxel_pop(executor);

    if (
        !callFunction || callFunction->type != VOXEL_TYPE_FUNCTION ||
        !callArgs || callArgs->type != VOXEL_TYPE_LIST || argCount < 2
    ) {
        return voxel_pushNull(executor);
    }

    voxel_FunctionType functionType = voxel_getFunctionType(executor->context, callFunction);

    if (functionType != VOXEL_FUNCTION_TYPE_POS_REF) {
        return voxel_pushNull(executor);
    }

    voxel_Executor* newExecutor = voxel_cloneExecutor(executor, VOXEL_FALSE);

    *voxel_getExecutorPosition(newExecutor) = (voxel_Position)(voxel_IntPtr)callFunction->value;

    newExecutor->scope = voxel_newScope(executor->context, executor->scope);

    voxel_List* argList = (voxel_List*)callArgs->value;
    voxel_ListItem* currentItem = argList->firstItem;
    voxel_Count callArgCount = 0;

    while (currentItem) {
        voxel_pushOntoList(newExecutor->context, newExecutor->valueStack, voxel_copyThing(executor->context, currentItem->value));

        currentItem = currentItem->nextItem;
        callArgCount++;
    }

    voxel_pushOntoList(newExecutor->context, newExecutor->valueStack, voxel_newNumberInt(newExecutor->context, callArgCount));

    voxel_push(executor, voxel_newNumberInt(executor->context, newExecutor->id));

    voxel_unreferenceThing(executor->context, callArgs);
    voxel_unreferenceThing(executor->context, callFunction);
}

void voxel_builtins_threads_destroyThread(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Count executorId = voxel_popNumberInt(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    if (!targetExecutor) {
        return voxel_pushNull(executor);
    }

    voxel_destroyExecutor(targetExecutor);

    voxel_pushNull(executor);
}

void voxel_builtins_threads_getOwnThreadId(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);

    voxel_push(executor, voxel_newNumberInt(executor->context, executor->id));
}

void voxel_builtins_threads_threadIsRunning(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Count executorId = voxel_popNumberInt(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    if (!targetExecutor) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newBoolean(executor->context, targetExecutor->isRunning));
}

void voxel_builtins_threads_setThreadIsRunning(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Count executorId = voxel_popNumberInt(executor);
    voxel_Bool isRunning = voxel_popBoolean(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    if (!targetExecutor) {
        return voxel_pushNull(executor);
    }

    targetExecutor->isRunning = isRunning;

    voxel_pushNull(executor);
}

void voxel_builtins_threads(voxel_Context* context) {
    voxel_defineBuiltin(context, ".Thn", &voxel_builtins_threads_newThread);
    voxel_defineBuiltin(context, ".Thd", &voxel_builtins_threads_destroyThread);
    voxel_defineBuiltin(context, ".Thoi", &voxel_builtins_threads_getOwnThreadId);
    voxel_defineBuiltin(context, ".Thir", &voxel_builtins_threads_threadIsRunning);
    voxel_defineBuiltin(context, ".Thsr", &voxel_builtins_threads_setThreadIsRunning);
}

#else

void voxel_builtins_threads(voxel_Context* context) {}

#endif