#ifdef VOXEL_BUILTINS_THREADS

void voxel_builtins_threads_newThread(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* callArgs = voxel_pop(executor);
    voxel_Thing* callFunction = voxel_pop(executor);

    VOXEL_REQUIRE(
        VOXEL_ARG(callFunction, VOXEL_TYPE_FUNCTION) &&
        VOXEL_ARG(callArgs, VOXEL_TYPE_LIST) &&
        voxel_getFunctionType(executor->context, callFunction) == VOXEL_FUNCTION_TYPE_POS_REF
    );

    voxel_Executor* newExecutor = voxel_cloneExecutor(executor, VOXEL_FALSE);

    *voxel_getExecutorPosition(newExecutor) = (voxel_Position)(voxel_IntPtr)callFunction->value;

    newExecutor->scope = voxel_newScope(executor->context, executor->scope);

    voxel_Thing* symbolsToPreserve = newExecutor->preserveSymbols;

    if (symbolsToPreserve && symbolsToPreserve->type == VOXEL_TYPE_LIST) {
        voxel_List* symbolList = (voxel_List*)symbolsToPreserve->value;
        voxel_ListItem* currentSymbol = symbolList->firstItem;

        while (currentSymbol) {
            voxel_ObjectItem* symbolValue = voxel_getScopeItem(newExecutor->scope, currentSymbol->value);

            if (symbolValue) {
                voxel_setLocalScopeItem(newExecutor->scope, currentSymbol->value, symbolValue->value);
            }

            currentSymbol = currentSymbol->nextItem;
        }
    }

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

    voxel_finally:

    voxel_unreferenceThing(executor->context, callArgs);
    voxel_unreferenceThing(executor->context, callFunction);
}

void voxel_builtins_threads_destroyThread(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Count executorId = voxel_popNumberInt(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    VOXEL_REQUIRE(targetExecutor);

    voxel_destroyExecutor(targetExecutor);

    voxel_pushNull(executor);

    voxel_finally:
}

void voxel_builtins_threads_getOwnThreadId(voxel_Executor* executor) {
    VOXEL_ARGC(0);

    voxel_push(executor, voxel_newNumberInt(executor->context, executor->id));

    voxel_finally:
}

void voxel_builtins_threads_threadIsRunning(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Count executorId = voxel_popNumberInt(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    VOXEL_REQUIRE(targetExecutor);

    voxel_push(executor, voxel_newBoolean(executor->context, targetExecutor->isRunning));

    voxel_finally:
}

void voxel_builtins_threads_setThreadIsRunning(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Count executorId = voxel_popNumberInt(executor);
    voxel_Bool isRunning = voxel_popBoolean(executor);

    voxel_Executor* targetExecutor = voxel_getExecutorById(executor->context, executorId);

    VOXEL_REQUIRE(targetExecutor);

    targetExecutor->isRunning = isRunning;

    voxel_pushNull(executor);

    voxel_finally:
}

void voxel_builtins_threads(voxel_Context* context) {
    voxel_defineBuiltin(context, ".threads_new", &voxel_builtins_threads_newThread);
    voxel_defineBuiltin(context, ".threads_destroy", &voxel_builtins_threads_destroyThread);
    voxel_defineBuiltin(context, ".threads_getOwnId", &voxel_builtins_threads_getOwnThreadId);
    voxel_defineBuiltin(context, ".threads_isRunning", &voxel_builtins_threads_threadIsRunning);
    voxel_defineBuiltin(context, ".threads_setIsRunning", &voxel_builtins_threads_setThreadIsRunning);
}

#else

void voxel_builtins_threads(voxel_Context* context) {}

#endif