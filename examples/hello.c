#include <stdio.h>
#include <libvoxel-config.h>
#include <libvoxel.h>

char* code = (char[]) {
    VOXEL_TOKEN_TYPE_NULL,
    VOXEL_TOKEN_TYPE_BOOLEAN_TRUE,
    VOXEL_TOKEN_TYPE_BOOLEAN_FALSE,
    VOXEL_TOKEN_TYPE_BYTE, 0x01,
    VOXEL_TOKEN_TYPE_BYTE, 0x02,
    VOXEL_TOKEN_TYPE_BYTE, 0x03,
    VOXEL_TOKEN_TYPE_NUMBER_INT_8, 0x04,
    VOXEL_TOKEN_TYPE_NUMBER_INT_16, 0x05, 0x06,
    VOXEL_TOKEN_TYPE_NUMBER_INT_32, 0x07, 0x08, 0x09, 0x0A,
    VOXEL_TOKEN_TYPE_NUMBER_FLOAT, 0x0B, 0x0C, 0x0D, 0x0E,
    VOXEL_TOKEN_TYPE_BUFFER_EMPTY, 0x00, 0x00, 0x01, 0x00,
    VOXEL_TOKEN_TYPE_BUFFER, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04,
    VOXEL_TOKEN_TYPE_STRING, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0',
    VOXEL_TOKEN_TYPE_CALL,
    0x00
};

int main(int argc, char* argv[]) {
    printf("Hello, world!\n");

    voxel_test();

    printf("New context\n");

    voxel_Context* context = voxel_newContext();

    context->code = code;
    context->codeLength = 55;

    while (VOXEL_TRUE) {
        VOXEL_ERRORABLE nextToken = voxel_nextToken(context); VOXEL_MUST_CODE(nextToken);

        if (nextToken.value == VOXEL_NULL) {
            break;
        }
    }

    printf("New null\n");

    voxel_Thing* null = voxel_newNull(context);

    printf("Unref\n");

    VOXEL_MUST_CODE(voxel_unreferenceThing(context, null));

    printf("It works!\n");

    return 0;
}