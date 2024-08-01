#include <stdio.h>
#include <string.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No input file specified\n");

        return 1;
    }

    voxel_Context* context = voxel_newContext();

    FILE* fp = fopen(argv[1], "r");

    if (!fp) {
        fprintf(stderr, "Error when reading file\n");

        return 1;
    }

    fseek(fp, 0, SEEK_END);

    voxel_Count size = ftell(fp);
    voxel_Byte* data = (voxel_Byte*)VOXEL_MALLOC(size + 1);

    fseek(fp, 0, SEEK_SET);

    if (fread(data, sizeof(voxel_Byte), size, fp) != size) {
        fprintf(stderr, "Error reading file contents\n");
        fclose(fp);

        VOXEL_FREE(data);

        return 1;
    }

    data[size] = '\0';

    context->code = data;
    context->codeSize = size;

    voxel_addArg(context, argv[0] != VOXEL_NULL ? argv[0] : "");

    voxel_Bool hasMetRuntimeOptionEnd = VOXEL_FALSE;

    for (voxel_Count i = 2; i < argc; i++) {
        if (!hasMetRuntimeOptionEnd && strcmp(argv[i], "--") == 0) {
            hasMetRuntimeOptionEnd = VOXEL_TRUE;

            continue;
        }

        voxel_addArg(context, argv[i]);
    }

    VOXEL_MUST_CODE(voxel_initContext(context));

    while (voxel_anyExecutorsRunning(context)) {
        VOXEL_MUST_CODE(voxel_stepContext(context));
    }

    return 0;
}
