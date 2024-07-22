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

void voxel_builtins_io(voxel_Context* context) {
    voxel_defineBuiltin(context, ".io_out", &voxel_builtins_io_out);
}

#else

void voxel_builtins_io(voxel_Context* context) {}

#endif