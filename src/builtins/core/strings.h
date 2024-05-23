#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_stringToNumber(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    if (!string) {
        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE result = voxel_stringToNumber(executor->context, string);

    voxel_unreferenceThing(executor->context, string);

    if (VOXEL_IS_ERROR(result)) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, result.value);
}

void voxel_builtins_core_getStringSize(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    if (!string) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getStringSize(string)));

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_appendToString(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* appendString = voxel_popString(executor);
    voxel_Thing* baseString = voxel_peek(executor, 0); // Keep as return value

    if (!appendString || !baseString || baseString->type != VOXEL_TYPE_STRING || argCount < 2) {
        return;
    }

    voxel_appendToString(executor->context, baseString, appendString);

    voxel_unreferenceThing(executor->context, appendString);
}

void voxel_builtins_core_reverseString(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || argCount < 1) {
        return;
    }

    voxel_reverseString(executor->context, string);
}

void voxel_builtins_core_cutStringStart(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int size = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || argCount < 2) {
        return;
    }

    if (size < 0) {
        return;
    }

    voxel_cutStringStart(executor->context, string, size);
}

void voxel_builtins_core_cutStringEnd(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int size = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || argCount < 2) {
        return;
    }

    if (size < 0) {
        return;
    }

    voxel_cutStringEnd(executor->context, string, size);
}

void voxel_builtins_core_padStringStart(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* fill = voxel_popString(executor);
    voxel_Int minSize = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!fill || !string || string->type != VOXEL_TYPE_STRING || argCount < 2) {
        return;
    }

    if (minSize < 0) {
        return;
    }

    voxel_padStringStart(executor->context, string, minSize, fill);

    voxel_unreferenceThing(executor->context, fill);
}

void voxel_builtins_core_padStringEnd(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* fill = voxel_popString(executor);
    voxel_Int minSize = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!fill || !string || string->type != VOXEL_TYPE_STRING || argCount < 2) {
        return;
    }

    if (minSize < 0) {
        return;
    }

    voxel_padStringEnd(executor->context, string, minSize, fill);

    voxel_unreferenceThing(executor->context, fill);
}

#endif