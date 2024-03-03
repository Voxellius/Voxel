#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

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
    if (argc < 2) {
        fprintf(stderr, "No input file specified\n");

        return 1;
    }

    voxel_Context* context = voxel_newContext();

    voxel_defineBuiltin(context, "log", &builtin_log);
    voxel_defineBuiltin(context, "add", &builtin_add);

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