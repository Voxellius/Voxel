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

_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_add, +);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_subtract, -);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_multiply, *);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_divide, /);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_lessThanOrEqualTo, <=);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_greaterThanOrEqualTo, >=);

void voxel_builtins_core_modulo(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int b = voxel_popNumberInt(executor);
    voxel_Int a = voxel_popNumberInt(executor);

    if (b == 0) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, a % b));
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
        voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, -voxel_getNumberFloat(value)));

    voxel_unreferenceThing(executor->context, value);
}

#endif