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

void voxel_copy(voxel_Byte* source, voxel_Byte* destination, voxel_Count size) {
    for (voxel_Count i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

// src/errors.h

#define VOXEL_ERROR_CODE int

#define VOXEL_ERRORABLE VOXEL_ERROR_CODE

#define VOXEL_ASSERT(condition, error) if (!(condition)) { \
        VOXEL_ERROR_MESSAGE("Voxel error: Assertion failed - ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (error); \
    }

#define VOXEL_THROW(error) do { \
        VOXEL_ERROR_MESSAGE("Voxel error: ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (error); \
    } while (0)

#define VOXEL_MUST(result) do { \
        int resultValue = (result); \
        if (resultValue) { \
            VOXEL_ERROR_MESSAGE("   ", "", __func__, __FILE__, __LINE__); \
            return resultValue; \
        } \
    } while (0)

#define VOXEL_OK 0
#define VOXEL_ERROR_NO_CODE -1
#define VOXEL_ERROR_TOKENISATION_BYTE -2
#define VOXEL_ERROR_TOKENISATION_END -3
#define VOXEL_ERROR_TYPE_MISMATCH -4
#define VOXEL_ERROR_NOT_IMPLEMENTED -5

const char* voxel_lookupError(VOXEL_ERROR_CODE error) {
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
    struct voxel_Token* tokens;
    voxel_Count tokenCount;
    voxel_Count currentPosition;
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
} voxel_Context;

voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context));

    context->code = NULL;
    context->tokens = NULL;
    context->tokenCount = 0;
    context->currentPosition = 0;
    context->firstTrackedThing = NULL;
    context->lastTrackedThing = NULL;

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

    while (currentThing != NULL) {
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
    thing->value = NULL;
    thing->referenceCount = 1;
    thing->previousTrackedThing = context->lastTrackedThing;
    thing->nextTrackedThing = NULL;

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
    VOXEL_ASSERT(thing->type == VOXEL_TYPE_NUMBER, VOXEL_ERROR_TYPE_MISMATCH);

    voxel_Number* number = thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return (voxel_Int)number->value.asFloat;
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

voxel_Float voxel_getNumberFloat(voxel_Thing* thing) {
    VOXEL_ASSERT(thing->type == VOXEL_TYPE_NUMBER, VOXEL_ERROR_TYPE_MISMATCH);

    voxel_Number* number = thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return (voxel_Float)number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return number->value.asFloat;
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
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

typedef struct voxel_TokenItem {
    voxel_Token token;
    struct voxel_TokenItem* nextTokenItem;
} voxel_TokenItem;

VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Count bytePosition, voxel_Count bytesToRead) {
    if (bytePosition + bytesToRead > context->codeLength) {
        VOXEL_THROW(VOXEL_ERROR_TOKENISATION_END);
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_tokenise(voxel_Context* context) {
    VOXEL_ASSERT(context->code, VOXEL_ERROR_NO_CODE);

    if (context->tokens) {
        // TODO: Free token data

        VOXEL_FREE(context->tokens);
    }

    voxel_TokenItem* firstTokenItem = NULL;
    voxel_TokenItem* currentTokenItem = NULL;
    voxel_Token tokenToAdd;
    voxel_Count tokenCount = 0;
    voxel_Count bytePosition = 0;

    while (VOXEL_TRUE) {
        VOXEL_MUST(voxel_safeToRead(context, bytePosition, 1));

        voxel_Bool shouldCreateToken = VOXEL_TRUE;
        voxel_Byte tokenType = context->code[bytePosition++];

        switch (tokenType) {
            case VOXEL_TOKEN_TYPE_NULL:
                tokenToAdd.data = voxel_newNull(context);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: null]\n");
                #endif

                break;

            case VOXEL_TOKEN_TYPE_TRUE:
            case VOXEL_TOKEN_TYPE_FALSE:
                tokenToAdd.data = voxel_newBoolean(context, tokenType == VOXEL_TOKEN_TYPE_TRUE);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: bool (");
                    VOXEL_LOG(tokenType == VOXEL_TOKEN_TYPE_TRUE ? "true" : "false");
                    VOXEL_LOG(")]\n");
                #endif

                break;

            case VOXEL_TOKEN_TYPE_BYTE:
                VOXEL_MUST(voxel_safeToRead(context, bytePosition, 1));

                tokenToAdd.data = voxel_newByte(context, context->code[bytePosition++]);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: byte]\n");
                #endif

                break;

            case VOXEL_TOKEN_TYPE_NUMBER_INT_8:
            case VOXEL_TOKEN_TYPE_NUMBER_INT_16:
            case VOXEL_TOKEN_TYPE_NUMBER_INT_32:
                VOXEL_MUST(voxel_safeToRead(context, bytePosition, 1));

                voxel_Int numberIntValue = context->code[bytePosition++];

                if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_16 || tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                    VOXEL_MUST(voxel_safeToRead(context, bytePosition, 1));

                    numberIntValue <<= 8; numberIntValue |= context->code[bytePosition++];
                }

                if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                    VOXEL_MUST(voxel_safeToRead(context, bytePosition, 2));

                    numberIntValue <<= 8; numberIntValue |= context->code[bytePosition++];
                    numberIntValue <<= 8; numberIntValue |= context->code[bytePosition++];
                }

                tokenToAdd.data = voxel_newNumberInt(context, numberIntValue);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: number (int)]\n");
                #endif

                break;

            case VOXEL_TOKEN_TYPE_NUMBER_FLOAT:
                VOXEL_MUST(voxel_safeToRead(context, bytePosition, 4));

                voxel_Float numberFloatValue;

                voxel_copy(&(context->code[bytePosition]), (char*)&numberFloatValue, 4);

                bytePosition += 4;
                tokenToAdd.data = voxel_newNumberFloat(context, numberFloatValue);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: number (float)]\n");
                #endif

                break;

            case '\0':
                shouldCreateToken = VOXEL_FALSE;

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Last byte]\n");
                #endif

                break;

            default:
                VOXEL_THROW(VOXEL_ERROR_TOKENISATION_BYTE);
        }

        tokenToAdd.type = tokenType;

        if (!shouldCreateToken) {
            break;
        }

        voxel_TokenItem* newTokenItem = VOXEL_MALLOC(sizeof(voxel_TokenItem));

        newTokenItem->token = tokenToAdd;
        newTokenItem->nextTokenItem = NULL;

        if (!firstTokenItem) {
            firstTokenItem = newTokenItem;
            currentTokenItem = newTokenItem;
        } else {
            currentTokenItem->nextTokenItem = newTokenItem;
            currentTokenItem = newTokenItem;
        }

        tokenCount++;
    }

    voxel_TokenItem* nextTokenItem;

    context->tokens = VOXEL_MALLOC(sizeof(voxel_Token) * tokenCount);
    currentTokenItem = firstTokenItem;

    for (voxel_Count i = 0; i < tokenCount; i++) {
        context->tokens[i] = currentTokenItem->token;
        nextTokenItem = currentTokenItem->nextTokenItem;

        VOXEL_FREE(currentTokenItem);

        currentTokenItem = nextTokenItem;
    }

    context->tokenCount = tokenCount;

    return VOXEL_OK;
}

// src/voxel.h

void voxel_test() {
    VOXEL_LOG("Hello from Voxel!\n");
}

#endif
