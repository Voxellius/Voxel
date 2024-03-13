VOXEL_ERRORABLE voxel_notOperation(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newBoolean(context, !voxel_thingIsTruthy(thing)));
}

VOXEL_ERRORABLE voxel_andOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    if (a->type == VOXEL_TYPE_BYTE) {
        VOXEL_ERRORABLE bByteResult = voxel_thingToByte(context, b); VOXEL_MUST(bByteResult);
        voxel_Thing* bByte = bByteResult.value;
        voxel_Thing* result = voxel_newByte(context, (voxel_IntPtr)a->value & (voxel_IntPtr)bByte->value);

        VOXEL_MUST(voxel_unreferenceThing(context, bByte));

        return VOXEL_OK_RET(result);
    }

    if (a->type == VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);
        voxel_Thing* bNumber = bNumberResult.value;

        VOXEL_MUST(voxel_unreferenceThing(context, bNumber));

        return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getNumberInt(a) & voxel_getNumberInt(bNumber)));
    }

    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) && voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_orOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    if (a->type == VOXEL_TYPE_BYTE) {
        VOXEL_ERRORABLE bByteResult = voxel_thingToByte(context, b); VOXEL_MUST(bByteResult);
        voxel_Thing* bByte = bByteResult.value;
        voxel_Thing* result = voxel_newByte(context, (voxel_IntPtr)a->value | (voxel_IntPtr)bByte->value);

        VOXEL_MUST(voxel_unreferenceThing(context, bByte));

        return VOXEL_OK_RET(result);
    }

    if (a->type == VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);
        voxel_Thing* bNumber = bNumberResult.value;

        VOXEL_MUST(voxel_unreferenceThing(context, bNumber));

        return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getNumberInt(a) | voxel_getNumberInt(bNumber)));
    }

    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) || voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_equalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_compareThings(a, b)));
}

VOXEL_ERRORABLE voxel_lessThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat(aNumberResult.value) < voxel_getNumberFloat(bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}

VOXEL_ERRORABLE voxel_greaterThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat(aNumberResult.value) > voxel_getNumberFloat(bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}