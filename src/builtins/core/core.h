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

void voxel_builtins_core(voxel_Context* context) {
    voxel_defineBuiltin(context, ".log", &voxel_builtins_core_log);
    voxel_defineBuiltin(context, ".+", &voxel_builtins_core_add);
    voxel_defineBuiltin(context, ".-", &voxel_builtins_core_subtract);
    voxel_defineBuiltin(context, ".*", &voxel_builtins_core_multiply);
    voxel_defineBuiltin(context, "./", &voxel_builtins_core_divide);
    voxel_defineBuiltin(context, ".%", &voxel_builtins_core_modulo);
    voxel_defineBuiltin(context, ".-x", &voxel_builtins_core_negate);
    voxel_defineBuiltin(context, ".<=", &voxel_builtins_core_lessThanOrEqualTo);
    voxel_defineBuiltin(context, ".>=", &voxel_builtins_core_greaterThanOrEqualTo);
    voxel_defineBuiltin(context, ".P", &voxel_builtins_core_params);
}

#else

void voxel_builtins_core(voxel_Context* context) {}

#endif