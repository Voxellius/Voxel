#ifndef LIBVOXEL_H_
#define LIBVOXEL_H_

// src/common.h

typedef VOXEL_BOOL voxel_Bool;
typedef VOXEL_COUNT voxel_Count;
typedef char voxel_Byte;
typedef VOXEL_INT_8 voxel_Int8;
typedef VOXEL_INT_16 voxel_Int16;
typedef VOXEL_INT_32 voxel_Int32;
typedef voxel_Int32 voxel_Int;
typedef VOXEL_FLOAT voxel_Float;

#define VOXEL_TRUE 1
#define VOXEL_FALSE 0
#define VOXEL_NULL 0

#define VOXEL_INTO_PTR(data, pointer) voxel_copy((voxel_Byte*)&(data), (voxel_Byte*)pointer, sizeof(data))

void voxel_copy(voxel_Byte* source, voxel_Byte* destination, voxel_Count size) {
    for (voxel_Count i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

// src/errors.h

typedef int voxel_ErrorCode;

#define VOXEL_ERRORABLE voxel_Result

#define VOXEL_ASSERT(condition, error) if (!(condition)) { \
        VOXEL_ERROR_MESSAGE("Voxel error: Assertion failed - ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (voxel_Result) {.errorCode = (error), .value = VOXEL_NULL}; \
    }

#define VOXEL_THROW(error) do { \
        VOXEL_ERROR_MESSAGE("Voxel error: ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (voxel_Result) {.errorCode = (error), .value = VOXEL_NULL}; \
    } while (0)

#define VOXEL_MUST(result) do { \
        voxel_Result storedResult = (result); \
        if (storedResult.errorCode != VOXEL_OK_CODE) { \
            VOXEL_ERROR_MESSAGE("   ", "", __func__, __FILE__, __LINE__); \
            return storedResult; \
        } \
    } while (0)

#define VOXEL_MUST_CODE(result) do { \
        voxel_Result storedResult = (result); \
        if (storedResult.errorCode != VOXEL_OK_CODE) { \
            VOXEL_ERROR_MESSAGE("   ", "", __func__, __FILE__, __LINE__); \
            return storedResult.errorCode; \
        } \
    } while (0)

#define VOXEL_IS_ERROR(result) ((result).errorCode != VOXEL_OK_CODE)

#define VOXEL_OK_CODE 0
#define VOXEL_ERROR_NO_CODE -1
#define VOXEL_ERROR_TOKENISATION_BYTE -2
#define VOXEL_ERROR_TOKENISATION_END -3
#define VOXEL_ERROR_TYPE_MISMATCH -4
#define VOXEL_ERROR_NOT_IMPLEMENTED -5

#define VOXEL_OK (voxel_Result) {.errorCode = VOXEL_OK_CODE, .value = VOXEL_NULL}
#define VOXEL_OK_RET(result) (voxel_Result) {.errorCode = VOXEL_OK_CODE, .value = (result)}

typedef struct voxel_Result {
    voxel_ErrorCode errorCode;
    void* value;
} voxel_Result;

const char* voxel_lookupError(voxel_ErrorCode error) {
    switch (error) {
        case VOXEL_ERROR_NO_CODE:
            return "No code loaded";

        case VOXEL_ERROR_TOKENISATION_BYTE:
            return "Unknown byte when tokenising";

        case VOXEL_ERROR_TOKENISATION_END:
            return "Unexpectedly reached end when tokenising";

        case VOXEL_ERROR_TYPE_MISMATCH:
            return "Type mismatch";

        case VOXEL_ERROR_NOT_IMPLEMENTED:
            return "Not implemented";

        default:
            return "Unknown error";
    }
}

// src/context.h

typedef struct voxel_Context {
    char* code;
    voxel_Count codeLength;
    voxel_Count currentPosition;
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
} voxel_Context;

voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context));

    context->code = VOXEL_NULL;
    context->currentPosition = 0;
    context->firstTrackedThing = VOXEL_NULL;
    context->lastTrackedThing = VOXEL_NULL;

    return context;
}

// src/things.h

typedef enum {
    VOXEL_TYPE_NULL,
    VOXEL_TYPE_BOOLEAN,
    VOXEL_TYPE_BYTE,
    VOXEL_TYPE_NUMBER,
    VOXEL_TYPE_BUFFER,
    VOXEL_TYPE_STRING,
    VOXEL_TYPE_NATIVE_FUNCTION,
    VOXEL_TYPE_DEFINED_FUNCTION
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* value;
    voxel_Count referenceCount;
    struct voxel_Thing* previousTrackedThing;
    struct voxel_Thing* nextTrackedThing;
} voxel_Thing;

void voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (thing->referenceCount > 0) {
        thing->referenceCount--;

        return;
    }

    if (thing == context->firstTrackedThing) {
        context->firstTrackedThing = thing->nextTrackedThing;
    }

    if (thing == context->lastTrackedThing) {
        context->lastTrackedThing = thing->previousTrackedThing;
    }

    if (thing->previousTrackedThing) {
        thing->previousTrackedThing->nextTrackedThing = thing->nextTrackedThing;
    }

    switch (thing->type) {
        case VOXEL_TYPE_NULL:
            break;

        default:
            VOXEL_FREE(thing->value);
    }

    VOXEL_FREE(thing);
}

void voxel_removeUnusedThings(voxel_Context* context) {
    voxel_Thing* currentThing = context->firstTrackedThing;

    while (currentThing != VOXEL_NULL) {
        voxel_Thing* nextThing = currentThing->nextTrackedThing;

        if (currentThing->referenceCount == 0) {
            voxel_unreferenceThing(context, currentThing);
        }

        currentThing = nextThing;
    }
}

voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = VOXEL_MALLOC(sizeof(voxel_Thing));

    thing->type = VOXEL_TYPE_NULL;
    thing->value = VOXEL_NULL;
    thing->referenceCount = 1;
    thing->previousTrackedThing = context->lastTrackedThing;
    thing->nextTrackedThing = VOXEL_NULL;

    if (context->lastTrackedThing) {
        context->lastTrackedThing->nextTrackedThing = thing;
    }

    context->lastTrackedThing = thing;

    if (!context->firstTrackedThing) {
        context->firstTrackedThing = thing;
    }

    return thing;
}

voxel_Thing* voxel_newNull(voxel_Context* context) {
    return voxel_newThing(context);
}

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x00 : (void*)0x01;

    return thing;
}

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(long)value;
}

// src/numbers.h

typedef enum {
    VOXEL_NUMBER_TYPE_INT,
    VOXEL_NUMBER_TYPE_FLOAT
} voxel_NumberType;

typedef struct voxel_Number {
    voxel_NumberType type;
    union {
        voxel_Int asInt;
        voxel_Float asFloat;
    } value;
} voxel_Number;

voxel_Thing* voxel_newNumberInt(voxel_Context* context, voxel_Int value) {
    voxel_Number* number = VOXEL_MALLOC(sizeof(voxel_Number));

    number->type = VOXEL_NUMBER_TYPE_INT;
    number->value.asInt = value;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = number;

    return thing;
}

voxel_Thing* voxel_newNumberFloat(voxel_Context* context, voxel_Float value) {
    voxel_Number* number = VOXEL_MALLOC(sizeof(voxel_Number));

    number->type = VOXEL_NUMBER_TYPE_FLOAT;
    number->value.asFloat = value;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = number;

    return thing;
}

voxel_Int voxel_getNumberInt(voxel_Thing* thing) {
    voxel_Number* number = thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return (voxel_Int)number->value.asFloat;
    }
}

voxel_Float voxel_getNumberFloat(voxel_Thing* thing) {
    voxel_Number* number = thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return (voxel_Float)number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return number->value.asFloat;
    }
}

// src/parser.h

typedef enum voxel_TokenType {
    VOXEL_TOKEN_TYPE_NULL = 0x01,
    VOXEL_TOKEN_TYPE_FALSE = 0x02,
    VOXEL_TOKEN_TYPE_TRUE = 0x03,
    VOXEL_TOKEN_TYPE_BYTE = 0x04,
    VOXEL_TOKEN_TYPE_NUMBER_INT_8 = 0x05,
    VOXEL_TOKEN_TYPE_NUMBER_INT_16 = 0x06,
    VOXEL_TOKEN_TYPE_NUMBER_INT_32 = 0x07,
    VOXEL_TOKEN_TYPE_NUMBER_FLOAT = 0x0F,
    VOXEL_TOKEN_TYPE_BUFFER = 0x10,
    VOXEL_TOKEN_TYPE_STRING = 0x11,
    VOXEL_TOKEN_TYPE_CALL = 0xFF
} voxel_TokenType;

typedef struct voxel_Token {
    voxel_TokenType type;
    void* data;
} voxel_Token;

VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Count bytesToRead) {
    if (context->currentPosition + bytesToRead > context->codeLength) {
        VOXEL_THROW(VOXEL_ERROR_TOKENISATION_END);
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_nextToken(voxel_Context* context) {
    VOXEL_ASSERT(context->code, VOXEL_ERROR_NO_CODE);
    
    voxel_Token token;

    VOXEL_MUST(voxel_safeToRead(context, 1));

    voxel_Bool shouldCreateToken = VOXEL_TRUE;
    voxel_Byte tokenType = context->code[context->currentPosition++];

    switch (tokenType) {
        case VOXEL_TOKEN_TYPE_NULL:
            token.data = voxel_newNull(context);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: null]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_TRUE:
        case VOXEL_TOKEN_TYPE_FALSE:
            token.data = voxel_newBoolean(context, tokenType == VOXEL_TOKEN_TYPE_TRUE);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: bool (");
                VOXEL_LOG(tokenType == VOXEL_TOKEN_TYPE_TRUE ? "true" : "false");
                VOXEL_LOG(")]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_BYTE:
            VOXEL_MUST(voxel_safeToRead(context, 1));

            token.data = voxel_newByte(context, context->code[context->currentPosition++]);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: byte]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_NUMBER_INT_8:
        case VOXEL_TOKEN_TYPE_NUMBER_INT_16:
        case VOXEL_TOKEN_TYPE_NUMBER_INT_32:
            VOXEL_MUST(voxel_safeToRead(context, 1));

            voxel_Int numberIntValue = context->code[context->currentPosition++];

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_16 || tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, 1));

                numberIntValue <<= 8; numberIntValue |= context->code[context->currentPosition++];
            }

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, 2));

                numberIntValue <<= 8; numberIntValue |= context->code[context->currentPosition++];
                numberIntValue <<= 8; numberIntValue |= context->code[context->currentPosition++];
            }

            token.data = voxel_newNumberInt(context, numberIntValue);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: num (int)]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_NUMBER_FLOAT:
            VOXEL_MUST(voxel_safeToRead(context, 4));

            voxel_Float numberFloatValue;

            voxel_copy(&(context->code[context->currentPosition]), (char*)&numberFloatValue, 4);

            context->currentPosition += 4;
            token.data = voxel_newNumberFloat(context, numberFloatValue);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: num (float)]\n");
            #endif

            break;

        case '\0':
            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Last byte]\n");
            #endif

            return VOXEL_OK_RET(VOXEL_NULL);

        default:
            VOXEL_THROW(VOXEL_ERROR_TOKENISATION_BYTE);
    }

    token.type = tokenType;

    voxel_Token* tokenPtr = VOXEL_MALLOC(sizeof(token));

    VOXEL_INTO_PTR(token, tokenPtr);

    return VOXEL_OK_RET(tokenPtr);
}

// src/voxel.h

void voxel_test() {
    VOXEL_LOG("Hello from Voxel!\n");
}

#endif
