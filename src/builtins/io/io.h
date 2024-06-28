#ifdef VOXEL_BUILTINS_IO

void voxel_builtins_io_out(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    voxel_pushNull(executor);
}

void voxel_builtins_io(voxel_Context* context) {
    voxel_defineBuiltin(context, ".io_out", &voxel_builtins_io_out);
}

#else

void voxel_builtins_io(voxel_Context* context) {}

#endif