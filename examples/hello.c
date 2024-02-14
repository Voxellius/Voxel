#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

char* code = (char[]) {
    VOXEL_TOKEN_TYPE_NULL,
    VOXEL_TOKEN_TYPE_TRUE,
    VOXEL_TOKEN_TYPE_FALSE,
    VOXEL_TOKEN_TYPE_BYTE, 0x01,
    VOXEL_TOKEN_TYPE_BYTE, 0x02,
    VOXEL_TOKEN_TYPE_BYTE, 0x03,
    VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0x04,
    VOXEL_TOKEN_TYPE_NUMBER_INT_16, 0x05, 0x06,
    VOXEL_TOKEN_TYPE_NUMBER_INT_32, 0x07, 0x08, 0x09, 0x0A,
    VOXEL_TOKEN_TYPE_NUMBER_FLOAT, 0x0B, 0x0C, 0x0D, 0x0E,
    0x00
};

int main(int argc, char* argv[]) {
    printf("Hello, world!\n");

    voxel_test();

    printf("New context\n");

    voxel_Context* context = voxel_newContext();

    context->code = code;
    context->codeLength = 25;

    while (VOXEL_TRUE) {
        VOXEL_ERRORABLE nextToken = voxel_nextToken(context); VOXEL_MUST_CODE(nextToken);

        if (nextToken.value == VOXEL_NULL) {
            break;
        }
    }

    printf("New null\n");

    voxel_Thing* null = voxel_newNull(context);

    printf("Unref\n");

    voxel_unreferenceThing(context, null);

    printf("It works!\n");

    return 0;
}