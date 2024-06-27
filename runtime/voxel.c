#include <stdio.h>
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

    size_t size = ftell(fp);
    char* data = (char*)VOXEL_MALLOC(size + 1);

    fseek(fp, 0, SEEK_SET);
    
    if (fread(data, sizeof(char), size, fp) != size) {
        fprintf(stderr, "Error reading file contents\n");
        fclose(fp);
        VOXEL_FREE(data);
        return 1;
    }

    context->code = data;
    context->codeLength = size;

    VOXEL_MUST_CODE(voxel_initContext(context));

    while (voxel_anyExecutorsRunning(context)) {
        VOXEL_MUST_CODE(voxel_stepContext(context));
    }

    return 0;
}
