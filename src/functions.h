voxel_Thing* voxel_newFunctionBuiltin(voxel_Context* context, voxel_Count builtinFunctionIndex) {
    voxel_Thing* thing = voxel_newThing(context);

    builtinFunctionIndex++;
    builtinFunctionIndex *= -1;

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)builtinFunctionIndex;

    return thing;
}

voxel_Thing* voxel_newFunctionPosRef(voxel_Context* context, voxel_Count positionReference) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)positionReference;

    return thing;
}

void voxel_destroyFunction(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareFunctions(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyFunction(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* newThing = voxel_newThing(context);

    newThing->type = VOXEL_TYPE_FUNCTION;
    newThing->value = thing->value;

    return newThing;
}

VOXEL_ERRORABLE voxel_functionToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(
        context,
        voxel_getFunctionType(context, thing) == VOXEL_FUNCTION_TYPE_BUILTIN ?
        "(builtin function)" :
        "(function)"
    ));
}

voxel_FunctionType voxel_getFunctionType(voxel_Context* context, voxel_Thing* thing) {
    if ((voxel_IntPtr)thing->value < 0) {
        return VOXEL_FUNCTION_TYPE_BUILTIN;
    }

    return VOXEL_FUNCTION_TYPE_POS_REF;
}