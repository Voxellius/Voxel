#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

// char* code = (char[]) {
//     // while (true) {
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_POS_REF_HERE,
//     // log("Hello, world!");
//     VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0',
//     VOXEL_TOKEN_TYPE_STRING, '.', 'l', 'o', 'g', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     // }
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_JUMP,
//     0x00
// };

char* code = (char[]) {
    // log("Hello, world!");
    VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0',
    VOXEL_TOKEN_TYPE_STRING, '.', 'l', 'o', 'g', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_CALL,
    // var x = 0;
    VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0,
    VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
    VOXEL_TOKEN_TYPE_SET,
    // while (true) {
    VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
    VOXEL_TOKEN_TYPE_POS_REF_HERE,
    // x = x + 1;
    VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0x01,
    VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_STRING, '.', 'a', 'd', 'd', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_CALL,
    VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
    VOXEL_TOKEN_TYPE_SET,
    // log(x);
    VOXEL_TOKEN_TYPE_STRING, '.', 'l', 'o', 'g', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_CALL,
    // }
    VOXEL_TOKEN_TYPE_NUMBER_INT_8, 100,
    VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_EQUAL,
    VOXEL_TOKEN_TYPE_NOT,
    VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY,
    0x00
};

void builtin_log(voxel_Executor* executor) {
    voxel_Thing* thing = voxel_pop(executor);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    // voxel_pushNull(executor);
}

void builtin_add(voxel_Executor* executor) {
    voxel_Thing* a = voxel_pop(executor);
    voxel_Thing* b = voxel_pop(executor);

    if (!a || !b) {
        voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, voxel_getNumberFloat(a) + voxel_getNumberFloat(b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

int main(int argc, char* argv[]) {
    printf("Hello, world!\n");

    voxel_test();

    printf("New context\n");

    voxel_Context* context = voxel_newContext();

    voxel_defineBuiltin(context, ".log", &builtin_log);
    voxel_defineBuiltin(context, ".add", &builtin_add);

    context->code = code;
    context->codeLength = 1024;

    voxel_Position currentPosition = 0;

    while (VOXEL_TRUE) {
        VOXEL_ERRORABLE nextToken = voxel_nextToken(context, &currentPosition); VOXEL_MUST_CODE(nextToken);

        if (nextToken.value == VOXEL_NULL) {
            break;
        }
    }

    while (voxel_anyContextsRunning(context)) {
        VOXEL_MUST_CODE(voxel_stepContext(context));
    }

    printf("It works!\n");

    return 0;
}