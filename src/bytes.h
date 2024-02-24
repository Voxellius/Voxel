voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(voxel_IntPtr)value;
}

VOXEL_ERRORABLE voxel_destroyByte(voxel_Thing* thing) {
    VOXEL_FREE(thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyByte(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newByte(context, (voxel_IntPtr)thing->value);
}

voxel_Thing* voxel_byteToNumber(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newNumberInt(context, (voxel_IntPtr)thing->value);
}

VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Byte bytes[1] = {(voxel_IntPtr)thing->value};

    return VOXEL_OK_RET(voxel_newString(context, 1, bytes));
}

VOXEL_ERRORABLE voxel_byteToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* string = voxel_newStringTerminated(context, "byte(0x");
    voxel_Thing* number = voxel_newNumberInt(context, (voxel_IntPtr)thing->value);
    VOXEL_ERRORABLE hexString = voxel_numberToBaseString(context, number, 16, 2); VOXEL_MUST(hexString);

    VOXEL_MUST(voxel_appendToString(context, string, hexString.value));
    VOXEL_MUST(voxel_appendByteToString(context, string, ')'));

    VOXEL_MUST(voxel_unreferenceThing(context, number));
    VOXEL_MUST(voxel_unreferenceThing(context, hexString.value));

    return VOXEL_OK_RET(string);
}