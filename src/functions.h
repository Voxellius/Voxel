voxel_Thing* voxel_newFunctionBuiltin(voxel_Context* context, voxel_Count builtinFunctionIndex) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

    builtinFunctionIndex++;
    builtinFunctionIndex *= -1;

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)builtinFunctionIndex;

    return thing;
}

voxel_Thing* voxel_newFunctionPosRef(voxel_Context* context, voxel_Position positionReference) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)positionReference;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyFunction(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_FUNCTION);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareFunctions(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyFunction(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* newThing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

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
    if ((voxel_Int)(voxel_IntPtr)thing->value < 0) {
        return VOXEL_FUNCTION_TYPE_BUILTIN;
    }

    return VOXEL_FUNCTION_TYPE_POS_REF;
}