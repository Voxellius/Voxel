voxel_Executor* voxel_newExecutor(voxel_Context* context) {
    voxel_Executor* executor = (voxel_Executor*)VOXEL_MALLOC(sizeof(voxel_Executor)); VOXEL_TAG_MALLOC(voxel_Executor);

    executor->context = context;
    executor->scope = context->globalScope;
    executor->preserveSymbols = VOXEL_NULL;
    executor->id = context->executorCount++;
    executor->isRunning = VOXEL_TRUE;
    executor->tokenisationState = VOXEL_STATE_NONE;
    executor->callStackSize = VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call);
    executor->callStack = (voxel_Call*)VOXEL_MALLOC(executor->callStackSize); VOXEL_TAG_MALLOC_SIZE("executor->callStack", VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call));
    executor->callStack[0] = (voxel_Call) {.position = VOXEL_MAGIC_SIZE, .canHandleExceptions = VOXEL_FALSE};
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

voxel_Executor* voxel_cloneExecutor(voxel_Executor* executor, voxel_Bool copyValueStack) {
    voxel_Context* context = executor->context;
    voxel_Executor* newExecutor = (voxel_Executor*)VOXEL_MALLOC(sizeof(voxel_Executor)); VOXEL_TAG_MALLOC(voxel_Executor);

    newExecutor->context = context;
    newExecutor->scope = executor->scope;
    newExecutor->preserveSymbols = executor->preserveSymbols;
    newExecutor->id = context->executorCount++;
    newExecutor->isRunning = VOXEL_TRUE;
    newExecutor->tokenisationState = VOXEL_STATE_NONE;
    newExecutor->callStackSize = VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call);
    newExecutor->callStack = (voxel_Call*)VOXEL_MALLOC(executor->callStackSize); VOXEL_TAG_MALLOC_SIZE("executor->callStack", VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call));
    newExecutor->callStack[0] = (voxel_Call) {.position = *voxel_getExecutorPosition(executor), .canHandleExceptions = VOXEL_FALSE};
    newExecutor->callStackHead = 0;
    newExecutor->valueStack = copyValueStack ? voxel_copyThing(context, executor->valueStack) : voxel_newList(context);
    newExecutor->previousExecutor = context->lastExecutor;
    newExecutor->nextExecutor = VOXEL_NULL;

    if (executor->preserveSymbols) {
        executor->preserveSymbols->referenceCount++;
    }

    if (!context->firstExecutor) {
        context->firstExecutor = newExecutor;
    }
    
    if (context->lastExecutor) {
        context->lastExecutor->nextExecutor = newExecutor;
    }

    context->lastExecutor = newExecutor;

    return newExecutor;
}

VOXEL_ERRORABLE voxel_destroyExecutor(voxel_Executor* executor) {
    VOXEL_FREE(executor->callStack); VOXEL_TAG_FREE_SIZE("voxel_Executor->callStack", executor->callStackSize);

    if (executor->scope != executor->context->globalScope) {
        VOXEL_MUST(voxel_destroyScope(executor->scope));
    }

    if (executor->preserveSymbols) {
        VOXEL_MUST(voxel_unreferenceThing(executor->context, executor->preserveSymbols));
    }

    voxel_List* valueStackList = (voxel_List*)executor->valueStack->value;
    voxel_ListItem* currentItem = valueStackList->firstItem;

    while (currentItem) {
        currentItem->value->referenceCount--;

        currentItem = currentItem->nextItem;
    }

    VOXEL_MUST(voxel_unreferenceThing(executor->context, executor->valueStack));

    if (executor == executor->context->firstExecutor) {
        executor->context->firstExecutor = executor->nextExecutor;
    }

    if (executor == executor->context->lastExecutor) {
        executor->context->lastExecutor = executor->previousExecutor;
    }

    if (executor->previousExecutor) {
        executor->previousExecutor->nextExecutor = executor->nextExecutor;
    }

    if (executor->nextExecutor) {
        executor->nextExecutor->previousExecutor = executor->previousExecutor;
    }

    VOXEL_FREE(executor); VOXEL_TAG_FREE(voxel_Executor);

    return VOXEL_OK;
}

voxel_Executor* voxel_getExecutorById(voxel_Context* context, voxel_Count id) {
    voxel_Executor* currentExecutor = context->firstExecutor;

    while (currentExecutor) {
        if (currentExecutor->id == id) {
            return currentExecutor;
        }

        currentExecutor = currentExecutor->nextExecutor;
    }

    return VOXEL_NULL;
}

voxel_Position* voxel_getExecutorPosition(voxel_Executor* executor) {
    return &executor->callStack[executor->callStackHead].position;
}

VOXEL_ERRORABLE voxel_stepExecutor(voxel_Executor* executor) {
    VOXEL_ASSERT(executor->context->isInitialised, VOXEL_ERROR_NOT_INITIALISED);

    if (!executor->isRunning) {
        return VOXEL_OK;
    }

    voxel_Position* position = voxel_getExecutorPosition(executor);
    voxel_Position oldPos = *position;
    VOXEL_ERRORABLE tokenResult = voxel_nextToken(executor, position); VOXEL_MUST(tokenResult);
    voxel_Token* token = (voxel_Token*)tokenResult.value;

    if (!token) {
        executor->isRunning = VOXEL_FALSE;

        return VOXEL_OK;
    }

    switch (token->type) {
        case VOXEL_TOKEN_TYPE_CALL:
        {
            VOXEL_ERRORABLE callThingResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(callThingResult);
            voxel_Thing* callThing = (voxel_Thing*)callThingResult.value;

            VOXEL_ASSERT(callThing, VOXEL_ERROR_MISSING_ARG);

            if (callThing->type == VOXEL_TYPE_FUNCTION) {
                voxel_FunctionType functionType = voxel_getFunctionType(executor->context, callThing);

                if (functionType == VOXEL_FUNCTION_TYPE_BUILTIN) {
                    voxel_Count builtinFunctionIndex = (voxel_IntPtr)callThing->value;

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

                voxel_stepInExecutor(executor, (voxel_Position)(voxel_IntPtr)callThing->value);
            } else if (callThing->type == VOXEL_TYPE_CLOSURE) {
                voxel_Closure* closure = (voxel_Closure*)callThing->value;

                voxel_stepInExecutor(executor, closure->position);

                voxel_Scope* scope = executor->scope;
                voxel_Thing* environment = closure->environment;
                voxel_Object* environmentObject = (voxel_Object*)environment->value;
                voxel_ObjectItem* currentItem = environmentObject->firstItem;

                while (currentItem) {
                    voxel_setObjectItem(executor->context, scope->things, currentItem->key, currentItem->value);

                    currentItem = currentItem->nextItem;
                }
            } else {
                VOXEL_THROW(VOXEL_ERROR_CANNOT_CALL_THING);
            }

            VOXEL_MUST(voxel_unreferenceThing(executor->context, callThing));

            break;
        }

        case VOXEL_TOKEN_TYPE_RETURN:
            voxel_stepOutExecutor(executor);
            break;

        case VOXEL_TOKEN_TYPE_THROW:
            VOXEL_MUST(voxel_throwException(executor));
            break;

        case VOXEL_TOKEN_TYPE_SET_HANDLER:
        {
            VOXEL_ERRORABLE handlerFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(handlerFunctionResult);
            voxel_Thing* handlerFunction = (voxel_Thing*)handlerFunctionResult.value;

            VOXEL_ASSERT(handlerFunction, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(handlerFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(voxel_getFunctionType(executor->context, handlerFunction) == VOXEL_FUNCTION_TYPE_POS_REF, VOXEL_ERROR_INVALID_ARG);

            voxel_setExceptionHandler(executor, (voxel_IntPtr)handlerFunction->value);

            VOXEL_MUST(voxel_unreferenceThing(executor->context, handlerFunction));

            break;
        }

        case VOXEL_TOKEN_TYPE_CLEAR_HANDLER:
            voxel_clearExceptionHandler(executor);
            break;

        case VOXEL_TOKEN_TYPE_GET:
        {
            VOXEL_ERRORABLE getKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(getKey);

            VOXEL_ASSERT(getKey.value, VOXEL_ERROR_MISSING_ARG);

            voxel_ObjectItem* scopeItem = voxel_getScopeItem(executor->scope, (voxel_Thing*)getKey.value);
            voxel_Thing* scopeValue = scopeItem ? (voxel_Thing*)scopeItem->value : voxel_newNull(executor->context);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, scopeValue));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, (voxel_Thing*)getKey.value));

            scopeValue->referenceCount++; // To ensure that builtins don't dereference the thing in the scope

            break;
        }

        case VOXEL_TOKEN_TYPE_SET:
        case VOXEL_TOKEN_TYPE_VAR:
        {
            VOXEL_ERRORABLE setKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(setKey);
            voxel_Thing* setValue = ((voxel_List*)executor->valueStack->value)->lastItem->value;

            VOXEL_ASSERT(setKey.value, VOXEL_ERROR_MISSING_ARG);
            VOXEL_ASSERT(setValue, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST((token->type == VOXEL_TOKEN_TYPE_VAR ? voxel_setLocalScopeItem : voxel_setScopeItem)(executor->scope, (voxel_Thing*)setKey.value, setValue));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, (voxel_Thing*)setKey.value));

            break;
        }

        case VOXEL_TOKEN_TYPE_POP:
        {
            VOXEL_ERRORABLE popResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(popResult);

            VOXEL_ASSERT(popResult.value, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST(voxel_unreferenceThing(executor->context, (voxel_Thing*)popResult.value));

            break;
        }

        case VOXEL_TOKEN_TYPE_DUPE:
        {
            voxel_Count dupeStackLength = voxel_getListLength(executor->valueStack);

            VOXEL_ASSERT(dupeStackLength > 0, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE dupeListItemResult = voxel_getListItem(executor->context, executor->valueStack, dupeStackLength - 1); VOXEL_MUST(dupeListItemResult);
            voxel_ListItem* dupeListItem = (voxel_ListItem*)dupeListItemResult.value;
            voxel_Thing* dupeThing = (voxel_Thing*)dupeListItem->value;

            dupeThing->referenceCount++;

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, dupeThing));

            break;
        }

        case VOXEL_TOKEN_TYPE_SWAP:
        {
            VOXEL_ERRORABLE swapBResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(swapBResult);
            VOXEL_ERRORABLE swapAResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(swapAResult);

            VOXEL_ASSERT(swapAResult.value, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(swapBResult.value, VOXEL_ERROR_INVALID_ARG);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, (voxel_Thing*)swapBResult.value));
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, (voxel_Thing*)swapAResult.value));

            break;
        }

        case VOXEL_TOKEN_TYPE_COPY:
        {
            voxel_Count copyStackLength = voxel_getListLength(executor->valueStack);

            VOXEL_ASSERT(copyStackLength > 0, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE copyListItemResult = voxel_getListItem(executor->context, executor->valueStack, copyStackLength - 1); VOXEL_MUST(copyListItemResult);
            voxel_ListItem* dupeListItem = (voxel_ListItem*)copyListItemResult.value;
            voxel_Thing* copyThing = (voxel_Thing*)dupeListItem->value;
            voxel_Thing* copiedThing = voxel_copyThing(executor->context, copyThing);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, copiedThing));

            break;
        }

        case VOXEL_TOKEN_TYPE_POS_REF_HERE:
        case VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE:
        case VOXEL_TOKEN_TYPE_POS_REF_BACKWARD:
        case VOXEL_TOKEN_TYPE_POS_REF_FORWARD:
        {
            VOXEL_ERRORABLE posRefKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(posRefKey);
            voxel_Position referencedPosition = *position;

            VOXEL_ASSERT(posRefKey.value, VOXEL_ERROR_INVALID_ARG);

            if (token->type == VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE) {
                referencedPosition = (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_BACKWARD) {
                referencedPosition -= (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_FORWARD) {
                referencedPosition += (voxel_IntPtr)token->data;
            }

            voxel_Thing* function = voxel_newFunctionPosRef(executor->context, referencedPosition);

            VOXEL_MUST(voxel_setScopeItem(executor->scope, (voxel_Thing*)posRefKey.value, function));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, (voxel_Thing*)posRefKey.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, function));

            break;
        }

        case VOXEL_TOKEN_TYPE_JUMP:
        case VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY:
        {
            VOXEL_ERRORABLE jumpFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpFunctionResult);
            voxel_Thing* jumpFunction = (voxel_Thing*)jumpFunctionResult.value;

            VOXEL_ASSERT(jumpFunction, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(jumpFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_CANNOT_JUMP_TO_THING);
            VOXEL_ASSERT(voxel_getFunctionType(executor->context, jumpFunction) == VOXEL_FUNCTION_TYPE_POS_REF, VOXEL_ERROR_CANNOT_JUMP_TO_THING);

            if (token->type == VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY) {
                VOXEL_ERRORABLE jumpConditionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpConditionResult);
                voxel_Thing* jumpCondition = (voxel_Thing*)jumpConditionResult.value;

                if (jumpCondition) {
                    voxel_Bool truthiness = voxel_thingIsTruthy(jumpCondition);

                    VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpCondition));

                    if (!truthiness) {
                        VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

                        break;
                    }
                }
            }

            *position = (voxel_IntPtr)jumpFunction->value;

            VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

            break;
        }

        case VOXEL_TOKEN_TYPE_NOT:
        {
            VOXEL_ERRORABLE notValueResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(notValueResult);

            VOXEL_ASSERT(notValueResult.value, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE notResult = voxel_notOperation(executor->context, (voxel_Thing*)notValueResult.value); VOXEL_MUST(notResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, (voxel_Thing*)notResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, (voxel_Thing*)notValueResult.value));

            break;
        }

        case VOXEL_TOKEN_TYPE_AND:
        case VOXEL_TOKEN_TYPE_OR:
        case VOXEL_TOKEN_TYPE_EQUAL:
        case VOXEL_TOKEN_TYPE_LESS_THAN:
        case VOXEL_TOKEN_TYPE_GREATER_THAN:
        {
            VOXEL_ERRORABLE binaryBResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryBResult);
            VOXEL_ERRORABLE binaryAResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryAResult);

            VOXEL_ASSERT(binaryAResult.value, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(binaryBResult.value, VOXEL_ERROR_INVALID_ARG);

            voxel_Thing* binaryB = (voxel_Thing*)binaryBResult.value;
            voxel_Thing* binaryA = (voxel_Thing*)binaryAResult.value;

            VOXEL_ERRORABLE binaryResult;

            if (token->type == VOXEL_TOKEN_TYPE_AND) {
                binaryResult = voxel_andOperation(executor->context, binaryA, binaryB);
            } else if (token->type == VOXEL_TOKEN_TYPE_OR) {
                binaryResult = voxel_orOperation(executor->context, binaryA, binaryB);
            } else if (token->type == VOXEL_TOKEN_TYPE_EQUAL) {
                binaryResult = voxel_equalOperation(executor->context, binaryA, binaryB);
            } else if (token->type == VOXEL_TOKEN_TYPE_LESS_THAN) {
                binaryResult = voxel_lessThanOperation(executor->context, binaryA, binaryB);
            } else if (token->type == VOXEL_TOKEN_TYPE_GREATER_THAN) {
                binaryResult = voxel_greaterThanOperation(executor->context, binaryA, binaryB);
            } else {
                VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
            }

            VOXEL_MUST(binaryResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, (voxel_Thing*)binaryResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryA));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryB));

            break;
        }

        default:
            // Token contains thing to be pushed onto value stack
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, (voxel_Thing*)token->data));
            break;
    }

    #ifdef VOXEL_DEBUG_EXECUTORS
        VOXEL_LOG("Current value stack: ");
        voxel_logThing(executor->context, executor->valueStack);
        VOXEL_LOG("\n");
    #endif

    VOXEL_FREE(token); VOXEL_TAG_FREE(voxel_Token);

    return VOXEL_OK;
}

void voxel_stepInExecutor(voxel_Executor* executor, voxel_Position position) {
    voxel_Scope* newScope = voxel_newScope(executor->context, executor->scope);

    executor->scope = newScope;

    executor->callStackHead++;

    voxel_Count neededSize = ((executor->callStackHead / VOXEL_CALL_STACK_BLOCK_LENGTH) + 1) * VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call);

    if (executor->callStackSize < neededSize) {
        executor->callStackSize = neededSize;
        executor->callStack = (voxel_Call*)VOXEL_REALLOC(executor->callStack, neededSize); VOXEL_TAG_REALLOC("voxel_Executor->callStack", neededSize - sizeof(voxel_Call), neededSize);
    }

    executor->callStack[executor->callStackHead] = (voxel_Call) {.position = position, .canHandleExceptions = VOXEL_FALSE};
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

void voxel_setExceptionHandler(voxel_Executor* executor, voxel_Position exceptionHandlerPosition) {
    voxel_Call* callStackTop = &executor->callStack[executor->callStackHead];

    callStackTop->canHandleExceptions = VOXEL_TRUE;
    callStackTop->exceptionHandlerPosition = exceptionHandlerPosition;
}

void voxel_clearExceptionHandler(voxel_Executor* executor) {
    executor->callStack[executor->callStackHead].canHandleExceptions = VOXEL_FALSE;
}

VOXEL_ERRORABLE voxel_throwException(voxel_Executor* executor) {
    while (!executor->callStack[executor->callStackHead].canHandleExceptions) {
        if (executor->callStackHead == 0) {
            #ifdef VOXEL_LOG_UNHANDLED_EXCEPTIONS
                voxel_Thing* valueStack = executor->valueStack;
                voxel_List* valueStackList = (voxel_List*)valueStack->value;
                voxel_ListItem* lastItem = valueStackList->lastItem;

                if (lastItem) {
                    VOXEL_LOG("Unhandled exception: ");
                    VOXEL_MUST(voxel_logThing(executor->context, lastItem->value));
                    VOXEL_LOG("\n");
                }
            #endif

            VOXEL_THROW(VOXEL_ERROR_UNHANDLED_EXCEPTION);
        }

        voxel_stepOutExecutor(executor);
    }

    *voxel_getExecutorPosition(executor) = executor->callStack[executor->callStackHead].exceptionHandlerPosition;

    voxel_clearExceptionHandler(executor);

    return VOXEL_OK;
}