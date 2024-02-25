#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

char* code = (char[]) {
    VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0',
    VOXEL_TOKEN_TYPE_STRING, '.', 'l', 'o', 'g', '\0',
    VOXEL_TOKEN_TYPE_GET,
    VOXEL_TOKEN_TYPE_CALL,
    0x00
};

void builtin_log(voxel_Executor* executor) {
    voxel_Thing* thing = voxel_pop(executor);

    if (thing) {
        voxel_logThing(executor->context, thing);
    }

    voxel_pushNull(executor);
}

int main(int argc, char* argv[]) {
    printf("Hello, world!\n");

    voxel_test();

    printf("New context\n");

    voxel_Context* context = voxel_newContext();

    voxel_defineBuiltin(context, ".log", &builtin_log);

    context->code = code;
    context->codeLength = 62;

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