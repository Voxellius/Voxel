#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

int main(int argc, char* argv[]) {
    printf("Hello, world!\n");

    voxel_test();

    printf("New context\n");

    voxel_Context* context = voxel_newContext();

    context->code = "nnn\0";

    VOXEL_MUST(voxel_tokenise(context));

    printf("Token count: %d\n", context->tokenCount);

    printf("New null\n");

    voxel_Thing* null = voxel_newNull(context);

    printf("Unref\n");

    voxel_unreferenceThing(context, null);

    printf("It works!\n");

    return 0;
}