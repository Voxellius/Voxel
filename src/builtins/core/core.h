#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_log(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_popNumber(executor);
    voxel_Thing* thing = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

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
        voxel_Thing* unusedThing = voxel_pop(executor);

        voxel_unreferenceThing(executor->context, unusedThing);

        actual--;
    }

    while (required > actual) {
        voxel_pushNull(executor);

        actual++;
    }
}

void voxel_builtins_core(voxel_Context* context) {
    voxel_defineBuiltin(context, ".log", &voxel_builtins_core_log);
    voxel_defineBuiltin(context, ".P", &voxel_builtins_core_params);

    voxel_defineBuiltin(context, ".+", &voxel_builtins_core_add);
    voxel_defineBuiltin(context, ".-", &voxel_builtins_core_subtract);
    voxel_defineBuiltin(context, ".*", &voxel_builtins_core_multiply);
    voxel_defineBuiltin(context, "./", &voxel_builtins_core_divide);
    voxel_defineBuiltin(context, ".%", &voxel_builtins_core_modulo);
    voxel_defineBuiltin(context, ".-x", &voxel_builtins_core_negate);
    voxel_defineBuiltin(context, ".<=", &voxel_builtins_core_lessThanOrEqualTo);
    voxel_defineBuiltin(context, ".>=", &voxel_builtins_core_greaterThanOrEqualTo);

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