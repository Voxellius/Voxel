#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_stringToNumber(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    VOXEL_ERRORABLE result = voxel_stringToNumber(executor->context, string);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(result));

    voxel_push(executor, (voxel_Thing*)result.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_stringToBuffer(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_push(executor, voxel_stringToBuffer(executor->context, string));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_getStringSize(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getStringSize(string)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_getStringLength(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getStringLength(string)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_stringCharIndexToByteIndex(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int charIndex = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_push(executor, voxel_newNumberInt(executor->context, (voxel_Int)voxel_stringCharIndexToByteIndex(string, charIndex)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_getStringByteRange(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Int end = voxel_popNumberInt(executor);
    voxel_Int start = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_push(executor, voxel_getStringByteRange(executor->context, string, start, end));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_getStringChar(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    if (index < 0) {
        index = voxel_getStringLength(string) + index;

        if (index < 0) {
            index = 0;
        }
    }

    voxel_Count start = voxel_stringCharIndexToByteIndex(string, index);
    voxel_Count end = voxel_stringCharIndexToByteIndex(string, index + 1);

    voxel_push(executor, voxel_getStringByteRange(executor->context, string, start, end));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_getStringByte(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_popString(executor);

    VOXEL_REQUIRE(string);

    voxel_String* stringValue = (voxel_String*)string->value;

    if (index < 0) {
        index = stringValue->size + index;

        if (index < 0) {
            index = 0;
        }
    }

    if (index >= stringValue->size) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_push(executor, voxel_newByte(executor->context, stringValue->value[index]));

    voxel_finally:

    voxel_unreferenceThing(executor->context, string);
}

void voxel_builtins_core_appendToString(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* appendString = voxel_popString(executor);
    voxel_Thing* baseString = voxel_peek(executor, 0); // Keep as return value

    if (!appendString || !baseString || baseString->type != VOXEL_TYPE_STRING || baseString->isLocked) {
        goto voxel_finally;
    }

    voxel_appendToString(executor->context, baseString, appendString);

    voxel_finally:

    voxel_unreferenceThing(executor->context, appendString);
}

void voxel_builtins_core_reverseString(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || string->isLocked) {
        goto voxel_finally;
    }

    voxel_reverseString(executor->context, string);

    voxel_finally:
}

void voxel_builtins_core_cutStringStart(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int size = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || string->isLocked) {
        goto voxel_finally;
    }

    if (size < 0) {
        goto voxel_finally;
    }

    voxel_cutStringStart(executor->context, string, size);

    voxel_finally:
}

void voxel_builtins_core_cutStringEnd(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int size = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!string || string->type != VOXEL_TYPE_STRING || string->isLocked) {
        goto voxel_finally;
    }

    if (size < 0) {
        goto voxel_finally;
    }

    voxel_cutStringEnd(executor->context, string, size);

    voxel_finally:
}

void voxel_builtins_core_padStringStart(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Thing* fill = voxel_popString(executor);
    voxel_Int minSize = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!fill || !string || string->type != VOXEL_TYPE_STRING || string->isLocked) {
        goto voxel_finally;
    }

    if (minSize < 0) {
        goto voxel_finally;
    }

    voxel_padStringStart(executor->context, string, minSize, fill);

    voxel_finally:

    voxel_unreferenceThing(executor->context, fill);
}

void voxel_builtins_core_padStringEnd(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Thing* fill = voxel_popString(executor);
    voxel_Int minSize = voxel_popNumberInt(executor);
    voxel_Thing* string = voxel_peek(executor, 0); // Keep as return value

    if (!fill || !string || string->type != VOXEL_TYPE_STRING || string->isLocked) {
        goto voxel_finally;
    }

    if (minSize < 0) {
        goto voxel_finally;
    }

    voxel_padStringEnd(executor->context, string, minSize, fill);

    voxel_finally:

    voxel_unreferenceThing(executor->context, fill);
}

#endif