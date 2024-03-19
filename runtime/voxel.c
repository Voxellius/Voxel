#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

#define RUNTIME_BUITLIN_NUMBER_OPERATOR(name, operator) void name(voxel_Executor* executor) { \
        voxel_Thing* argCount = voxel_pop(executor); \
        voxel_Thing* b = voxel_pop(executor); \
        voxel_Thing* a = voxel_pop(executor); \
\
        voxel_unreferenceThing(executor->context, argCount); \
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

void builtin_log(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* thing = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    voxel_pushNull(executor);
}

RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_add, +);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_subtract, -);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_multiply, *);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_divide, /);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_lessThanOrEqualTo, <=);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_lessThan, <);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_greaterThanOrEqualTo, >=);
RUNTIME_BUITLIN_NUMBER_OPERATOR(builtin_greaterThan, >);

void builtin_modulo(voxel_Executor* executor) { 
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (!a || !b) {
        voxel_pushNull(executor);

        return;
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, voxel_getNumberInt(a) % voxel_getNumberInt(b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void builtin_equal(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (!a || !b) {
        voxel_pushNull(executor);

        return;
    }

    voxel_push(executor, voxel_newBoolean(executor->context, voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void builtin_notEqual(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (!a || !b) {
        voxel_pushNull(executor);

        return;
    }

    voxel_push(executor, voxel_newBoolean(executor->context, !voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void builtin_negate(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* value = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (!value) {
        voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, -voxel_getNumberFloat(value)));

    voxel_unreferenceThing(executor->context, value);
}

void builtin_params(voxel_Executor* executor) {
    voxel_Thing* requiredThing = voxel_pop(executor);
    voxel_Thing* actualThing = voxel_pop(executor);

    if (!requiredThing || !actualThing) {
        return;
    }

    voxel_Int required = voxel_getNumberInt(requiredThing);
    voxel_Int actual = voxel_getNumberInt(actualThing);

    while (required < actual) {
        voxel_Thing* unusedThing = voxel_pop(executor);

        voxel_unreferenceThing(executor->context, unusedThing);

        actual--;
    }

    while (required > actual) {
        voxel_pushNull(executor);

        actual++;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No input file specified\n");

        return 1;
    }

    voxel_Context* context = voxel_newContext();

    voxel_defineBuiltin(context, "log", &builtin_log);
    voxel_defineBuiltin(context, ".log", &builtin_log);
    voxel_defineBuiltin(context, ".+", &builtin_add);
    voxel_defineBuiltin(context, ".-", &builtin_subtract);
    voxel_defineBuiltin(context, ".*", &builtin_multiply);
    voxel_defineBuiltin(context, "./", &builtin_divide);
    voxel_defineBuiltin(context, ".%", &builtin_modulo);
    voxel_defineBuiltin(context, ".-x", &builtin_negate);
    voxel_defineBuiltin(context, ".<=", &builtin_lessThanOrEqualTo);
    voxel_defineBuiltin(context, ".<", &builtin_lessThan);
    voxel_defineBuiltin(context, ".>=", &builtin_greaterThanOrEqualTo);
    voxel_defineBuiltin(context, ".>", &builtin_greaterThan);
    voxel_defineBuiltin(context, ".==", &builtin_equal);
    voxel_defineBuiltin(context, ".!=", &builtin_notEqual);
    voxel_defineBuiltin(context, ".P", &builtin_params);

    FILE* fp = fopen(argv[1], "r");

    if (!fp) {
        fprintf(stderr, "Error when reading file\n");

        return 1;
    }

    fseek(fp, 0, SEEK_END);

    size_t size = ftell(fp);
    char* data = VOXEL_MALLOC(size + 1);

    fseek(fp, 0, SEEK_SET);
    fread(data, sizeof(char), size, fp);

    context->code = data;
    context->codeLength = size;

    VOXEL_MUST_CODE(voxel_initContext(context));

    while (voxel_anyContextsRunning(context)) {
        VOXEL_MUST_CODE(voxel_stepContext(context));
    }

    return 0;
}