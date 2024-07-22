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
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwiseLeftShift, <<);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwiseRightShift, >>);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_and, &);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_xor, ^);
_VOXEL_BUILTINS_CORE_NUMBER_INT_OPERATOR(voxel_builtins_core_bitwise_or, |);

void voxel_builtins_core_add(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (a->type != VOXEL_TYPE_NUMBER || b->type != VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE aStringResult = voxel_thingToString(executor->context, a);
        VOXEL_ERRORABLE bStringResult = voxel_thingToString(executor->context, b);

        VOXEL_REQUIRE(!VOXEL_IS_ERROR(aStringResult) && !VOXEL_IS_ERROR(bStringResult));

        voxel_Thing* aString = (voxel_Thing*)aStringResult.value;
        voxel_Thing* bString = (voxel_Thing*)bStringResult.value;

        voxel_push(executor, voxel_concatenateStrings(executor->context, aString, bString));

        voxel_unreferenceThing(executor->context, aString);
        voxel_unreferenceThing(executor->context, bString);

        goto voxel_finally;
    }

    voxel_Float aFloat = voxel_getNumberFloat(a);
    voxel_Float bFloat = voxel_getNumberFloat(b);

    voxel_push(executor, voxel_newNumberFloat(executor->context, aFloat + bFloat));

    voxel_finally:

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_modulo(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Int b = voxel_popNumberInt(executor);
    voxel_Int a = voxel_popNumberInt(executor);

    VOXEL_REQUIRE(b != 0);

    voxel_push(executor, voxel_newNumberInt(executor->context, a % b));

    voxel_finally:
}

void voxel_builtins_core_bitwiseUnsignedRightShift(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* b = voxel_popNumber(executor);
    voxel_Thing* a = voxel_popNumber(executor);

    VOXEL_REQUIRE(a && b);

    voxel_push(executor, voxel_newNumberInt(executor->context, (voxel_UInt)voxel_getNumberInt(a) >> voxel_getNumberInt(b)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_equal(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    VOXEL_REQUIRE(a && b);

    voxel_push(executor, voxel_newBoolean(executor->context, voxel_compareThings(a, b)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_notEqual(voxel_Executor* executor) {
    VOXEL_ARGC(2);

    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    VOXEL_REQUIRE(a && b);

    voxel_push(executor, voxel_newBoolean(executor->context, !voxel_compareThings(a, b)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_negate(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* value = voxel_popNumber(executor);

    VOXEL_REQUIRE(value);

    voxel_push(executor, voxel_newNumberFloat(executor->context, -voxel_getNumberFloat(value)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, value);
}

void voxel_builtins_core_bitwiseNot(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* value = voxel_popNumber(executor);

    VOXEL_REQUIRE(value);

    voxel_push(executor, voxel_newNumberInt(executor->context, ~voxel_getNumberInt(value)));

    voxel_finally:

    voxel_unreferenceThing(executor->context, value);
}

void voxel_builtins_core_increment(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* thing = voxel_popNumber(executor);

    VOXEL_REQUIRE(thing && !thing->isLocked);

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

    voxel_finally:
}

void voxel_builtins_core_decrement(voxel_Executor* executor) {
    VOXEL_ARGC(1);

    voxel_Thing* thing = voxel_popNumber(executor);

    VOXEL_REQUIRE(thing && !thing->isLocked);

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

    voxel_finally:
}

#endif