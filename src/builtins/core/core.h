#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_log(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    voxel_pushNull(executor);
}

void voxel_builtins_core_params(voxel_Executor* executor) {
    voxel_Int required = voxel_popNumberInt(executor);
    voxel_Int actual = voxel_popNumberInt(executor);

    while (required < actual) {
        voxel_popVoid(executor);

        actual--;
    }

    while (required > actual) {
        voxel_pushNull(executor);

        actual++;
    }
}

void voxel_builtins_core_getType(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (!thing) {
        return voxel_pushNull(executor);
    }

    voxel_Byte thingType[2] = {0x00, 0x00};

    switch (thing->type) {
        case VOXEL_TYPE_NULL: thingType[0] = 'n'; break;
        case VOXEL_TYPE_BOOLEAN: thingType[0] = 't'; break;
        case VOXEL_TYPE_BYTE: thingType[0] = 'b'; break;
        case VOXEL_TYPE_FUNCTION: thingType[0] = '@'; break;
        case VOXEL_TYPE_CLOSURE: thingType[0] = 'C'; break;
        case VOXEL_TYPE_NUMBER: thingType[0] = '%'; break;
        case VOXEL_TYPE_BUFFER: thingType[0] = 'B'; break;
        case VOXEL_TYPE_STRING: thingType[0] = '"'; break;
        case VOXEL_TYPE_OBJECT: thingType[0] = 'O'; break;
        case VOXEL_TYPE_LIST: thingType[0] = 'L'; break;
    }

    voxel_unreferenceThing(executor->context, thing);

    voxel_push(executor, voxel_newStringTerminated(executor->context, thingType));
}

void voxel_builtins_core_toClosure(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* environment = voxel_pop(executor);
    voxel_Thing* function = voxel_pop(executor);

    if (
        !environment || environment->type != VOXEL_TYPE_OBJECT ||
        !function || function->type != VOXEL_TYPE_FUNCTION ||
        argCount < 2
    ) {
        return voxel_pushNull(executor);
    }

    if (voxel_getFunctionType(executor->context, function) != VOXEL_FUNCTION_TYPE_POS_REF) {
        voxel_unreferenceThing(executor->context, environment);
        voxel_unreferenceThing(executor->context, function);

        return voxel_pushNull(executor);
    }

    voxel_push(
        executor,
        voxel_newClosure(executor->context, (voxel_Position)(voxel_IntPtr)function->value, environment)
    );

    voxel_unreferenceThing(executor->context, environment);
    voxel_unreferenceThing(executor->context, function);
}

void voxel_builtins_core_pushArgs(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return;
    }

    voxel_List* listValue = list->value;
    voxel_ListItem* currentItem = listValue->firstItem;

    while (currentItem) {
        voxel_Thing* currentThing = currentItem->value;

        voxel_push(executor, currentThing);

        currentThing->referenceCount++;

        currentItem = currentItem->nextItem;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_getItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 2) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_getObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_getListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_setItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 3) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_setObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_setListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Value
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_removeItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 2) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_removeObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_removeListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_getLength(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 1) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 1);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_getObjectLength(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_getListLength(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing

    voxel_pushNull(executor);
}

void voxel_builtins_core(voxel_Context* context) {
    voxel_defineBuiltin(context, ".log", &voxel_builtins_core_log);
    voxel_defineBuiltin(context, ".P", &voxel_builtins_core_params);
    voxel_defineBuiltin(context, ".T", &voxel_builtins_core_getType);
    voxel_defineBuiltin(context, ".C", &voxel_builtins_core_toClosure);
    voxel_defineBuiltin(context, ".Au", &voxel_builtins_core_pushArgs);

    voxel_defineBuiltin(context, ".+", &voxel_builtins_core_add);
    voxel_defineBuiltin(context, ".-", &voxel_builtins_core_subtract);
    voxel_defineBuiltin(context, ".*", &voxel_builtins_core_multiply);
    voxel_defineBuiltin(context, "./", &voxel_builtins_core_divide);
    voxel_defineBuiltin(context, ".%", &voxel_builtins_core_modulo);
    voxel_defineBuiltin(context, ".-x", &voxel_builtins_core_negate);
    voxel_defineBuiltin(context, ".<=", &voxel_builtins_core_lessThanOrEqualTo);
    voxel_defineBuiltin(context, ".>=", &voxel_builtins_core_greaterThanOrEqualTo);
    voxel_defineBuiltin(context, ".++", &voxel_builtins_core_increment);
    voxel_defineBuiltin(context, ".--", &voxel_builtins_core_decrement);

    voxel_defineBuiltin(context, ".Tg", &voxel_builtins_core_getItem);
    voxel_defineBuiltin(context, ".Ts", &voxel_builtins_core_setItem);
    voxel_defineBuiltin(context, ".Tr", &voxel_builtins_core_removeItem);
    voxel_defineBuiltin(context, ".Tl", &voxel_builtins_core_getLength);

    voxel_defineBuiltin(context, ".O", &voxel_builtins_core_newObject);
    voxel_defineBuiltin(context, ".Og", &voxel_builtins_core_getObjectItem);
    voxel_defineBuiltin(context, ".Os", &voxel_builtins_core_setObjectItem);
    voxel_defineBuiltin(context, ".Or", &voxel_builtins_core_removeObjectItem);
    voxel_defineBuiltin(context, ".Ogg", &voxel_builtins_core_getObjectItemGetter);
    voxel_defineBuiltin(context, ".Osg", &voxel_builtins_core_setObjectItemGetter);
    voxel_defineBuiltin(context, ".Ogs", &voxel_builtins_core_getObjectItemSetter);
    voxel_defineBuiltin(context, ".Oss", &voxel_builtins_core_setObjectItemSetter);
    voxel_defineBuiltin(context, ".Ol", &voxel_builtins_core_getObjectLength);
    voxel_defineBuiltin(context, ".Op", &voxel_builtins_core_getObjectPrototypes);

    voxel_defineBuiltin(context, ".L", &voxel_builtins_core_newList);
    voxel_defineBuiltin(context, ".Lo", &voxel_builtins_core_newListOf);
    voxel_defineBuiltin(context, ".Lg", &voxel_builtins_core_getListItem);
    voxel_defineBuiltin(context, ".Ls", &voxel_builtins_core_setListItem);
    voxel_defineBuiltin(context, ".Lr", &voxel_builtins_core_removeListItem);
    voxel_defineBuiltin(context, ".Lu", &voxel_builtins_core_pushOntoList);
    voxel_defineBuiltin(context, ".Lp", &voxel_builtins_core_popFromList);
    voxel_defineBuiltin(context, ".Li", &voxel_builtins_core_insertIntoList);
    voxel_defineBuiltin(context, ".Ll", &voxel_builtins_core_getListLength);
    voxel_defineBuiltin(context, ".Lj", &voxel_builtins_core_joinList);
}

#else

void voxel_builtins_core(voxel_Context* context) {}

#endif