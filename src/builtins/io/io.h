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

    voxel_Byte* pathString = voxel_getString(path);
    voxel_Byte* modeString;

    switch (mode) {
        case 'r': modeString = "rb"; break;
        case 'w': modeString = "wb"; break;
        default: VOXEL_FAIL(); break;
    }

    FILE* fp = fopen(pathString, modeString);

    if (!fp) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_Handle* handle = voxel_openHandle(executor->context, &voxel_builtins_io_handle, fp);

    voxel_push(executor, voxel_newNumberInt(executor->context, handle->id));

    voxel_finally:

    VOXEL_FREE(pathString); VOXEL_TAG_FREE(voxel_Byte*);

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

void voxel_builtins_io_seek(voxel_Executor* executor) {
    VOXEL_ARGC(3);

    voxel_Bool seekRelative = voxel_popBoolean(executor);
    voxel_Int seekBy = voxel_popNumberInt(executor);
    voxel_Count handleId = voxel_popNumberInt(executor);

    voxel_Handle* handle = voxel_getHandleById(executor->context, handleId);

    VOXEL_REQUIRE(handle);

    FILE* fp = (FILE*)handle->value;

    voxel_Bool isSuccess = VOXEL_FALSE;

    if (!seekRelative && seekBy < 0) {
        isSuccess = fseek(fp, 0, SEEK_END) == 0;

        seekBy += 1;

        if (seekBy < 0) {
            isSuccess &= fseek(fp, seekBy, SEEK_CUR) == 0;
        }
    } else {
        isSuccess = fseek(fp, seekBy, seekRelative ? SEEK_CUR : SEEK_SET) == 0;
    }

    if (isSuccess) {
        voxel_push(executor, voxel_newNumberInt(executor->context, ftell(fp)));
    } else {
        voxel_pushNull(executor);
    }

    voxel_finally:
}

void voxel_builtins_io_read(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int size = voxel_popNumberInt(executor);
    voxel_Int handleId = voxel_popNumberInt(executor);

    voxel_Handle* handle = voxel_getHandleById(executor->context, handleId);

    VOXEL_REQUIRE(size >= 0 && handle);

    if (size > VOXEL_MAX_BUFFER_WRITE_BLOCK) {
        size = VOXEL_MAX_BUFFER_WRITE_BLOCK;
    }

    voxel_Thing* buffer = voxel_newBuffer(executor->context, size, VOXEL_NULL);
    voxel_Buffer* bufferValue = (voxel_Buffer*)buffer->value;

    if (size == 0) {
        voxel_push(executor, buffer);

        goto voxel_finally;
    }

    voxel_Count bytesRead = fread(bufferValue->value, sizeof(voxel_Byte), size, (FILE*)handle->value);

    if (bytesRead == 0) {
        voxel_pushNull(executor);

        goto voxel_finally;
    }

    voxel_push(executor, buffer);

    voxel_finally:
}

void voxel_builtins_io_write(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int handleId = voxel_popNumberInt(executor);
    voxel_Thing* buffer = voxel_pop(executor);

    voxel_Handle* handle = voxel_getHandleById(executor->context, handleId);

    VOXEL_REQUIRE(VOXEL_ARG(buffer, VOXEL_TYPE_BUFFER) && handle);

    voxel_Buffer* bufferValue = (voxel_Buffer*)buffer->value;

    voxel_Count size = bufferValue->size;

    if (size == 0) {
        voxel_push(executor, voxel_newNumberInt(executor->context, 0));

        goto voxel_finally;
    }

    if (size > VOXEL_MAX_BUFFER_WRITE_BLOCK) {
        size = VOXEL_MAX_BUFFER_WRITE_BLOCK;
    }

    voxel_Count bytesWritten = fwrite(bufferValue->value, sizeof(voxel_Byte), size, (FILE*)handle->value);

    if (bytesWritten == 0) {
        voxel_push(executor, voxel_newNumberInt(executor->context, -1));

        goto voxel_finally;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, bytesWritten));

    voxel_finally:

    voxel_unreferenceThing(executor->context, buffer);
}

#endif

void voxel_builtins_io(voxel_Context* context) {
    voxel_defineBuiltin(context, ".io_out", &voxel_builtins_io_out);

    #ifdef VOXEL_USE_STDLIB
        voxel_defineBuiltin(context, ".io_open", &voxel_builtins_io_open);
        voxel_defineBuiltin(context, ".io_close", &voxel_builtins_io_close);
        voxel_defineBuiltin(context, ".io_seek", &voxel_builtins_io_seek);
        voxel_defineBuiltin(context, ".io_read", &voxel_builtins_io_read);
        voxel_defineBuiltin(context, ".io_write", &voxel_builtins_io_write);
    #endif
}

#else

void voxel_builtins_io(voxel_Context* context) {}

#endif