voxel_Executor* voxel_newExecutor(voxel_Context* context) {
    voxel_Executor* executor = VOXEL_MALLOC(sizeof(voxel_Executor)); VOXEL_TAG_MALLOC(voxel_Executor);

    executor->context = context;
    executor->scope = voxel_newScope(context, context->globalScope);
    executor->isRunning = VOXEL_TRUE;
    executor->callStackSize = VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Count);
    executor->callStack = VOXEL_MALLOC(executor->callStackSize); VOXEL_TAG_MALLOC_SIZE("executor->callStack", VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Count));
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

            VOXEL_MUST(voxel_unreferenceThing(executor->context, callFunction));

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
            VOXEL_MUST(voxel_unreferenceThing(executor->context, getKey.value));

            scopeValue->referenceCount++; // To ensure that builtins don't dereference the thing in the scope

            break;

        case VOXEL_TOKEN_TYPE_SET:
            VOXEL_ERRORABLE setKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(setKey);
            voxel_Thing* setValue = ((voxel_List*)executor->valueStack->value)->lastItem->value;

            VOXEL_ASSERT(setKey.value, VOXEL_ERROR_MISSING_ARG);
            VOXEL_ASSERT(setValue, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST(voxel_setScopeItem(executor->scope, setKey.value, setValue));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, setKey.value));

            break;

        case VOXEL_TOKEN_TYPE_POS_REF_HERE:
        case VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE:
        case VOXEL_TOKEN_TYPE_POS_REF_BACKWARD:
        case VOXEL_TOKEN_TYPE_POS_REF_FORWARD:
            VOXEL_ERRORABLE posRefKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(posRefKey);
            voxel_Position referencedPosition = *position;

            VOXEL_ASSERT(posRefKey.value, VOXEL_ERROR_INVALID_ARGUMENT);

            if (token->type == VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE) {
                referencedPosition = (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_BACKWARD) {
                referencedPosition -= (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_FORWARD) {
                referencedPosition += (voxel_IntPtr)token->data;
            }

            voxel_Thing* function = voxel_newFunctionPosRef(executor->context, referencedPosition);

            VOXEL_MUST(voxel_setScopeItem(executor->scope, posRefKey.value, function));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, posRefKey.value));

            break;

        case VOXEL_TOKEN_TYPE_JUMP:
        case VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY:
            VOXEL_ERRORABLE jumpFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpFunctionResult);
            voxel_Thing* jumpFunction = jumpFunctionResult.value;

            VOXEL_ASSERT(jumpFunction, VOXEL_ERROR_INVALID_ARGUMENT);
            VOXEL_ASSERT(jumpFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_CANNOT_JUMP_TO_THING);
            VOXEL_ASSERT(voxel_getFunctionType(executor->context, jumpFunction) == VOXEL_FUNCTION_TYPE_POS_REF, VOXEL_ERROR_CANNOT_JUMP_TO_THING);

            if (token->type == VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY) {
                VOXEL_ERRORABLE jumpConditionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpConditionResult);
                voxel_Thing* jumpCondition = jumpConditionResult.value;

                if (jumpCondition) {
                    voxel_Bool truthiness = voxel_thingIsTruthy(jumpCondition);

                    VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpCondition));

                    if (!voxel_thingIsTruthy(jumpCondition)) {
                        VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

                        break;
                    }
                }
            }

            *position = (voxel_IntPtr)jumpFunction->value;

            VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

            break;

        case VOXEL_TOKEN_TYPE_NOT:
            VOXEL_ERRORABLE notValueResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(notValueResult);

            VOXEL_ASSERT(notValueResult.value, VOXEL_ERROR_INVALID_ARGUMENT);

            VOXEL_ERRORABLE notResult = voxel_notOperation(executor->context, notValueResult.value); VOXEL_MUST(notResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, notResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, notValueResult.value));

            break;

        case VOXEL_TOKEN_TYPE_AND:
        case VOXEL_TOKEN_TYPE_OR:
        case VOXEL_TOKEN_TYPE_EQUAL:
            VOXEL_ERRORABLE binaryBResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryBResult);
            VOXEL_ERRORABLE binaryAResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryAResult);

            VOXEL_ASSERT(binaryAResult.value, VOXEL_ERROR_INVALID_ARGUMENT);
            VOXEL_ASSERT(binaryBResult.value, VOXEL_ERROR_INVALID_ARGUMENT);

            VOXEL_ERRORABLE binaryResult;

            if (token->type == VOXEL_TOKEN_TYPE_AND) {
                binaryResult = voxel_andOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_OR) {
                binaryResult = voxel_orOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_EQUAL) {
                binaryResult = voxel_equalOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else {
                VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
            }

            VOXEL_MUST(binaryResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, binaryResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryAResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryBResult.value));

            break;

        case VOXEL_TOKEN_TYPE_LESS_THAN:
        case VOXEL_TOKEN_TYPE_GREATER_THAN:
            VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);

        default:
            // Token contains thing to be pushed onto value stack
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, token->data));
            break;
    }

    #ifdef VOXEL_DEBUG_EXECUTORS
        VOXEL_DEBUG_LOG("Current value stack: ");
        voxel_logThing(executor->context, executor->valueStack);
    #endif

    VOXEL_FREE(token); VOXEL_TAG_FREE(voxel_Token);

    return VOXEL_OK;
}

void voxel_stepInExecutor(voxel_Executor* executor, voxel_Position position) {
    voxel_Scope* newScope = voxel_newScope(executor->context, executor->scope);

    executor->scope = newScope;

    executor->callStackHead++;

    voxel_Count neededSize = ((executor->callStackHead / VOXEL_CALL_STACK_BLOCK_LENGTH) + 1) * sizeof(voxel_Count);

    if (executor->callStackSize < neededSize) {
        executor->callStackSize = neededSize;
        executor->callStack = VOXEL_REALLOC(executor->callStack, neededSize); VOXEL_TAG_REALLOC("voxel_Executor->callStack", neededSize - sizeof(voxel_Count), neededSize);
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