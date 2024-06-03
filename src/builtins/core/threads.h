#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newThread(voxel_Executor* executor) {
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

    voxel_Executor* newExecutor = voxel_cloneExecutor(executor);

    *voxel_getExecutorPosition(newExecutor) = (voxel_Position)(voxel_IntPtr)callFunction->value;

    newExecutor->scope = voxel_newScope(executor->context, executor->scope);

    voxel_List* argList = (voxel_List*)callArgs->value;
    voxel_ListItem* currentItem = argList->firstItem;
    voxel_Count callArgCount = 0;

    while (currentItem) {
        voxel_pushOntoList(newExecutor->context, newExecutor->valueStack, currentItem->value);

        currentItem->value->referenceCount++;

        currentItem = currentItem->nextItem;
        callArgCount++;
    }

    voxel_pushOntoList(newExecutor->context, newExecutor->valueStack, voxel_newNumberInt(newExecutor->context, callArgCount));

    voxel_push(executor, voxel_newNumberInt(executor->context, newExecutor->id));

    voxel_unreferenceThing(executor->context, callArgs);
}

#endif