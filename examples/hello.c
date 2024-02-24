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

    printf("Log code Vxon\n");

    voxel_Thing* buffer = voxel_newBuffer(context, context->codeLength, context->code);
    VOXEL_ERRORABLE bufferVxon = voxel_thingToVxon(context, buffer); VOXEL_MUST_CODE(bufferVxon);

    voxel_logString(bufferVxon.value);

    printf("\n");

    printf("Log number\n");

    voxel_Thing* number = voxel_newNumberFloat(context, -123456.789012);
    VOXEL_ERRORABLE numberString = voxel_thingToString(context, number); VOXEL_MUST_CODE(numberString);

    voxel_logString(numberString.value);

    printf("\n");

    printf("Log byte Vxon\n");

    voxel_Thing* byte = voxel_newByte(context, 'V');
    VOXEL_ERRORABLE byteVxon = voxel_thingToVxon(context, byte); VOXEL_MUST_CODE(byteVxon);

    voxel_logString(byteVxon.value);

    printf("\n");

    printf("New strings\n");

    voxel_Thing* hello = voxel_newStringTerminated(context, "Hello!\n");
    voxel_Thing* abc = voxel_newString(context, 3, "abc");
    voxel_Thing* def = voxel_newString(context, 3, "def");
    voxel_Thing* ghi = voxel_newString(context, 3, "ghi");
    voxel_Thing* jkl = voxel_newString(context, 3, "jkl");
    voxel_Thing* mno = voxel_newString(context, 3, "mno");
    voxel_Thing* newline = voxel_newStringTerminated(context, "\n");

    voxel_logString(hello);

    // printf("Concatenate strings\n");

    voxel_Thing* concat = voxel_newStringTerminated(context, "Testing! ");

    VOXEL_MUST_CODE(voxel_appendToString(context, concat, hello));

    // voxel_logString(concat);

    printf("New object\n");

    voxel_Thing* object = voxel_newObject(context);

    printf("Set object properties\n");

    VOXEL_MUST_CODE(voxel_setObjectItem(context, object, abc, def));
    VOXEL_MUST_CODE(voxel_setObjectItem(context, object, def, null));
    VOXEL_MUST_CODE(voxel_setObjectItem(context, object, ghi, buffer));
    VOXEL_MUST_CODE(voxel_setObjectItem(context, object, jkl, number));
    VOXEL_MUST_CODE(voxel_setObjectItem(context, object, mno, byte));

    printf("Log object Vxon\n");

    VOXEL_ERRORABLE objectVxon = voxel_thingToVxon(context, object); VOXEL_MUST_CODE(objectVxon);

    voxel_logString(objectVxon.value);

    printf("\n");

    printf("It works!\n");

    return 0;
}