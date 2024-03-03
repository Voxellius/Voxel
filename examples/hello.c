#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

// char* code = (char[]) {
//     #ifdef VOXEL_MAGIC
//         VOXEL_MAGIC,
//     #endif
//     // while (true) {
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_POS_REF_HERE,
//     // log("Hello, world!");
//     VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0',
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 1,
//     VOXEL_TOKEN_TYPE_STRING, 'l', 'o', 'g', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     // }
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_JUMP,
//     0x00
// };

// char* code = (char[]) {
//     #ifdef VOXEL_MAGIC
//         VOXEL_MAGIC,
//     #endif
//     // log("Hello, world!\n");
//     VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\n', '\0',
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 1,
//     VOXEL_TOKEN_TYPE_STRING, 'l', 'o', 'g', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     // var x = 0;
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0,
//     VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
//     VOXEL_TOKEN_TYPE_SET,
//     // while (true) {
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_POS_REF_HERE,
//     // x = x + 1;
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0x01,
//     VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 2,
//     VOXEL_TOKEN_TYPE_STRING, 'a', 'd', 'd', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
//     VOXEL_TOKEN_TYPE_SET,
//     // log(x);
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 1,
//     VOXEL_TOKEN_TYPE_STRING, 'l', 'o', 'g', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     // log(byte("\n"));
//     VOXEL_TOKEN_TYPE_BYTE, '\n',
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 1,
//     VOXEL_TOKEN_TYPE_STRING, 'l', 'o', 'g', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_CALL,
//     // }
//     VOXEL_TOKEN_TYPE_NUMBER_INT_8, 100,
//     VOXEL_TOKEN_TYPE_STRING, 'x', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_EQUAL,
//     VOXEL_TOKEN_TYPE_NOT,
//     VOXEL_TOKEN_TYPE_STRING, 'a', '\0',
//     VOXEL_TOKEN_TYPE_GET,
//     VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY,
//     0x00
// };

char* code = (char[]) {
    #ifdef VOXEL_MAGIC
        VOXEL_MAGIC,
    #endif
    34,  84, 104, 105, 115,  32, 109, 101, 115,
  115,  97, 103, 101,  32, 119,  97, 115,  32,
  102, 114, 111, 109,  32,  97,  32,  46, 118,
  120, 108,  32, 102, 105, 108, 101,  33,  10,
    0,  51,   1,  34, 108, 111, 103,   0,  63,
   33
};

void builtin_log(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* thing = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    // voxel_pushNull(executor);
}

void builtin_add(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);
    voxel_Thing* b = voxel_pop(executor);

    voxel_unreferenceThing(executor->context, argCount);

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

    voxel_defineBuiltin(context, "log", &builtin_log);
    voxel_defineBuiltin(context, "add", &builtin_add);

    context->code = code;
    context->codeLength = 1024;

    voxel_initContext(context);

    voxel_Position currentPosition = 4;

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