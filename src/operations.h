VOXEL_ERRORABLE voxel_notOperation(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newBoolean(context, !voxel_thingIsTruthy(thing)));
}

VOXEL_ERRORABLE voxel_andOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) && voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_orOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) || voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_identicalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, a == b));
}

VOXEL_ERRORABLE voxel_equalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_compareThings(a, b)));
}

VOXEL_ERRORABLE voxel_lessThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat((voxel_Thing*)aNumberResult.value) < voxel_getNumberFloat((voxel_Thing*)bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, (voxel_Thing*)aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, (voxel_Thing*)bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}

VOXEL_ERRORABLE voxel_greaterThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat((voxel_Thing*)aNumberResult.value) > voxel_getNumberFloat((voxel_Thing*)bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, (voxel_Thing*)aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, (voxel_Thing*)bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}