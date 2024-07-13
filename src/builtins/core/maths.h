#ifdef VOXEL_BUILTINS_CORE

#define _VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(name, operator) void name(voxel_Executor* executor) { \
        voxel_Int argCount = voxel_popNumberInt(executor); \
        voxel_Thing* b = voxel_popNumber(executor); \
        voxel_Thing* a = voxel_popNumber(executor); \
\
        if (!a || !b) { \
            voxel_pushNull(executor); \
\
            return; \
        } \
\
        voxel_push(executor, voxel_newNumberFloat(executor->context, voxel_getNumberFloat(a) operator voxel_getNumberFloat(b))); \
\
        voxel_unreferenceThing(executor->context, a); \
        voxel_unreferenceThing(executor->context, b); \
    }

#define _VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(name, operator) void name(voxel_Executor* executor) { \
        voxel_Int argCount = voxel_popNumberInt(executor); \
        voxel_Thing* b = voxel_popNumber(executor); \
        voxel_Thing* a = voxel_popNumber(executor); \
\
        if (!a || !b) { \
            voxel_pushNull(executor); \
\
            return; \
        } \
\
        voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getNumberInt(a) operator voxel_getNumberInt(b))); \
\
        voxel_unreferenceThing(executor->context, a); \
        voxel_unreferenceThing(executor->context, b); \
    }

_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_subtract, -);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_multiply, *);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_divide, /);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_lessThanOrEqualTo, <=);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_greaterThanOrEqualTo, >=);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_left_shift, <<);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_right_shift, >>);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_and, &);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_xor, ^);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_or, |);

void voxel_builtins_core_add(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (a->type != VOXEL_TYPE_NUMBER || b->type != VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE aStringResult = voxel_thingToString(executor->context, a);
        VOXEL_ERRORABLE bStringResult = voxel_thingToString(executor->context, b);

        if (VOXEL_IS_ERROR(aStringResult) || VOXEL_IS_ERROR(bStringResult)) {
            return voxel_pushNull(executor);
        }

        voxel_Thing* aString = (voxel_Thing*)aStringResult.value;
        voxel_Thing* bString = (voxel_Thing*)bStringResult.value;

        voxel_unreferenceThing(executor->context, a);
        voxel_unreferenceThing(executor->context, b);

        voxel_push(executor, voxel_concatenateStrings(executor->context, aString, bString));

        voxel_unreferenceThing(executor->context, aString);
        voxel_unreferenceThing(executor->context, bString);

        return;
    }

    voxel_Float aFloat = voxel_getNumberFloat(a);
    voxel_Float bFloat = voxel_getNumberFloat(b);

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);

    voxel_push(executor, voxel_newNumberFloat(executor->context, aFloat + bFloat));
}

void voxel_builtins_core_modulo(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int b = voxel_popNumberInt(executor);
    voxel_Int a = voxel_popNumberInt(executor);

    if (b == 0) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, a % b));
}

void voxel_builtins_core_bitwise_unsigned_right_shift(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_popNumber(executor);
    voxel_Thing* a = voxel_popNumber(executor);

    if (!a || !b) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, (voxel_UInt)voxel_getNumberInt(a) >> voxel_getNumberInt(b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_equal(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (!a || !b) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newBoolean(executor->context, voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_notEqual(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (!a || !b) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newBoolean(executor->context, !voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_negate(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* value = voxel_popNumber(executor);

    if (!value) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, -voxel_getNumberFloat(value)));

    voxel_unreferenceThing(executor->context, value);
}

void voxel_builtins_core_increment(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_popNumber(executor);

    if (!thing || thing->isLocked) {
        return voxel_pushNull(executor);
    }

    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            number->value.asInt++;
            break;

        case VOXEL_NUMBER_TYPE_FLOAT:
            number->value.asFloat++;
            break;
    }

    voxel_push(executor, thing);
}

void voxel_builtins_core_decrement(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_popNumber(executor);

    if (!thing || thing->isLocked) {
        return voxel_pushNull(executor);
    }

    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            number->value.asInt--;
            break;

        case VOXEL_NUMBER_TYPE_FLOAT:
            number->value.asFloat--;
            break;
    }

    voxel_push(executor, thing);
}

#endif