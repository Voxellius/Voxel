#ifdef VOXEL_BUILTINS_IO

void voxel_builtins_io_out(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* thing = voxel_pop(executor);

    VOXEL_REQUIRE(thing);

    voxel_logThing(executor->context, thing);

    voxel_pushNull(executor);

    voxel_finally:

    voxel_unreferenceThing(executor->context, thing);
}

#ifdef VOXEL_USE_STDLIB

void voxel_builtins_io_closeHandle(voxel_Handle* handle) {
    fclose((FILE*)handle->value);
}

voxel_HandleType voxel_builtins_io_handle = {
    .closer = voxel_builtins_io_closeHandle
};

void voxel_builtins_io_open(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Byte mode = voxel_popByteValue(executor);
    voxel_Thing* path = voxel_popString(executor);

    VOXEL_REQUIRE(path);

    voxel_String* pathValue = (voxel_String*)path->value;
    voxel_Byte* modeString;

    switch (mode) {
        case 'r': modeString = "rb"; break;
        case 'w': modeString = "wb"; break;
        default: VOXEL_FAIL(); break;
    }

    FILE* fp = fopen(pathValue->value, modeString);

    if (!fp) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Handle* handle = voxel_openHandle(executor->context, &voxel_builtins_io_handle, fp);

    voxel_push(executor, voxel_newNumberInt(executor->context, handle->id));

    voxel_finally:

    voxel_unreferenceThing(executor->context, path);
}

void voxel_builtins_io_close(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Count handleId = voxel_popNumberInt(executor);

    voxel_Handle* handle = voxel_getHandleById(executor->context, handleId);
    
    VOXEL_REQUIRE(handle);

    voxel_closeHandle(executor->context, handle);

    voxel_pushNull(executor);

    voxel_finally:
}

#endif

void voxel_builtins_io(voxel_Context* context) {
    voxel_defineBuiltin(context, ".io_out", &voxel_builtins_io_out);

    #ifdef VOXEL_USE_STDLIB
        voxel_defineBuiltin(context, ".io_open", &voxel_builtins_io_open);
        voxel_defineBuiltin(context, ".io_close", &voxel_builtins_io_close);
    #endif
}

#else

void voxel_builtins_io(voxel_Context* context) {}

#endif