#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_numberToString(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* number = voxel_popNumber(executor);

    VOXEL_REQUIRE(number);

    VOXEL_ERRORABLE conversionResult = voxel_numberToString(executor->context, number);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(conversionResult));

    voxel_push(executor, (voxel_Thing*)conversionResult.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, number);
}

void voxel_builtins_core_numberToBaseString(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int base = voxel_popNumberInt(executor);
    voxel_Thing* number = voxel_popNumber(executor);

    VOXEL_REQUIRE(number);

    VOXEL_ERRORABLE conversionResult = voxel_numberToBaseString(executor->context, number, base, 1);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(conversionResult));

    voxel_push(executor, (voxel_Thing*)conversionResult.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, number);
}

void voxel_builtins_core_numberToByte(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* number = voxel_popNumber(executor);

    VOXEL_REQUIRE(number);

    VOXEL_ERRORABLE conversionResult = voxel_numberToByte(executor->context, number);

    VOXEL_REQUIRE(!VOXEL_IS_ERROR(conversionResult));

    voxel_push(executor, (voxel_Thing*)conversionResult.value);

    voxel_finally:

    voxel_unreferenceThing(executor->context, number);
}

void voxel_builtins_core_isNan(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* number = voxel_popNumber(executor);

    VOXEL_REQUIRE(number);

    voxel_push(executor, voxel_newBoolean(executor->context, voxel_isNan(number)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, number);
}

#endif