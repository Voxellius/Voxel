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
typedef VOXEL_UINT_8 voxel_UInt8;
typedef VOXEL_UINT_16 voxel_UInt16;
typedef VOXEL_UINT_32 voxel_UInt32;
typedef voxel_UInt32 voxel_UInt;
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
    VOXEL_TYPE_STRING
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* value;
    voxel_Count referenceCount;
    struct voxel_Thing* previousTrackedThing;
    struct voxel_Thing* nextTrackedThing;
} voxel_Thing;

void voxel_destroyNull(voxel_Thing* thing);
void voxel_destroyBoolean(voxel_Thing* thing);
void voxel_destroyByte(voxel_Thing* thing);
void voxel_destroyNumber(voxel_Thing* thing);
void voxel_destroyBuffer(voxel_Thing* thing);
void voxel_destroyString(voxel_Thing* thing);

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

void voxel_destroyNull(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x00 : (void*)0x01;

    return thing;
}

void voxel_destroyBoolean(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(long)value;
}

void voxel_destroyByte(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

VOXEL_ERRORABLE voxel_destroyThing(voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: voxel_destroyNull(thing); return VOXEL_OK;
        case VOXEL_TYPE_BOOLEAN: voxel_destroyBoolean(thing); return VOXEL_OK;
        case VOXEL_TYPE_BYTE: voxel_destroyByte(thing); return VOXEL_OK;
        case VOXEL_TYPE_NUMBER: voxel_destroyNumber(thing); return VOXEL_OK;
        case VOXEL_TYPE_BUFFER: voxel_destroyBuffer(thing); return VOXEL_OK;
        case VOXEL_TYPE_STRING: voxel_destroyString(thing); return VOXEL_OK;
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (thing->referenceCount > 0) {
        thing->referenceCount--;

        return VOXEL_OK;
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

    VOXEL_MUST(voxel_destroyThing(thing));
}

VOXEL_ERRORABLE voxel_removeUnusedThings(voxel_Context* context) {
    voxel_Thing* currentThing = context->firstTrackedThing;

    while (currentThing != VOXEL_NULL) {
        voxel_Thing* nextThing = currentThing->nextTrackedThing;

        if (currentThing->referenceCount == 0) {
            VOXEL_MUST(voxel_unreferenceThing(context, currentThing));
        }

        currentThing = nextThing;
    }

    return VOXEL_OK;
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

void voxel_destroyNumber(voxel_Thing* thing) {
    VOXEL_FREE(thing->value);
    VOXEL_FREE(thing);
}

// src/buffers.h

typedef struct voxel_Buffer {
    voxel_Count size;
    voxel_Byte* value;
} voxel_Buffer;

voxel_Thing* voxel_newBuffer(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_Buffer* buffer = VOXEL_MALLOC(sizeof(voxel_Buffer));

    buffer->size = size;
    buffer->value = VOXEL_MALLOC(size);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BUFFER;

    if (data != VOXEL_NULL) {
        voxel_copy(data, buffer->value, size);
    } else {
        for (voxel_Count i = 0; i < size; i++) {
            buffer->value[i] = 0;
        }
    }

    return thing;
}

void voxel_destroyBuffer(voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    VOXEL_FREE(buffer->value);
    VOXEL_FREE(buffer);
    VOXEL_FREE(thing);
}

// src/strings.h

typedef struct voxel_String {
    voxel_Count length;
    voxel_Byte* value;
} voxel_String;

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count length, voxel_Byte* data) {
    voxel_String* string = VOXEL_MALLOC(sizeof(voxel_String));

    string->length = length;
    string->value = VOXEL_MALLOC(length);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;

    if (length > 0) {
        voxel_copy(data, string->value, length);
    }

    return thing;
}

void voxel_destroyString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    VOXEL_FREE(string->value);
    VOXEL_FREE(string);
    VOXEL_FREE(thing);
}

// src/parser.h

typedef enum voxel_TokenType {
    VOXEL_TOKEN_TYPE_NULL = 'n',
    VOXEL_TOKEN_TYPE_BOOLEAN_TRUE = 't',
    VOXEL_TOKEN_TYPE_BOOLEAN_FALSE = 'f',
    VOXEL_TOKEN_TYPE_BYTE = 'b',
    VOXEL_TOKEN_TYPE_NUMBER_INT_8 = '3',
    VOXEL_TOKEN_TYPE_NUMBER_INT_16 = '4',
    VOXEL_TOKEN_TYPE_NUMBER_INT_32 = '5',
    VOXEL_TOKEN_TYPE_NUMBER_FLOAT = '%',
    VOXEL_TOKEN_TYPE_BUFFER = 'B',
    VOXEL_TOKEN_TYPE_BUFFER_EMPTY = 'E',
    VOXEL_TOKEN_TYPE_STRING = '$',
    VOXEL_TOKEN_TYPE_CALL = '!'
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

        case VOXEL_TOKEN_TYPE_BOOLEAN_TRUE:
        case VOXEL_TOKEN_TYPE_BOOLEAN_FALSE:
            token.data = voxel_newBoolean(context, tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: bool (");
                VOXEL_LOG(tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE ? "true" : "false");
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

        case VOXEL_TOKEN_TYPE_BUFFER:
        case VOXEL_TOKEN_TYPE_BUFFER_EMPTY:
            VOXEL_MUST(voxel_safeToRead(context, 4));

            voxel_UInt32 size = 0;

            for (voxel_Count i = 0; i < 4; i++) {
                size <<= 8;
                size |= context->code[context->currentPosition++];
            }

            if (tokenType == VOXEL_TOKEN_TYPE_BUFFER_EMPTY) {
                token.data = voxel_newBuffer(context, size, VOXEL_NULL);

                #ifdef VOXEL_DEBUG
                    VOXEL_LOG("[Token: buffer (empty)]\n");
                #endif

                break;
            }

            VOXEL_MUST(voxel_safeToRead(context, size));

            token.data = voxel_newBuffer(context, size, &(context->code[context->currentPosition]));
            context->currentPosition += size;

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: buffer (declared)]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_STRING:
            voxel_Byte currentByte = '\0';
            voxel_Byte* currentString = NULL;
            voxel_Count currentSize = 0;
            voxel_Count length = 0;

            while (VOXEL_TRUE) {
                VOXEL_MUST(voxel_safeToRead(context, 1));

                currentByte = context->code[context->currentPosition++];

                if (currentByte == '\0') {
                    break;
                }

                voxel_Count neededSize = (((length / VOXEL_STRING_BLOCK_SIZE) + 1) * VOXEL_STRING_BLOCK_SIZE);

                if (currentString == NULL) {
                    currentString = VOXEL_MALLOC(neededSize);
                    currentSize = neededSize;
                } else if (neededSize > currentSize) {
                    currentString = VOXEL_REALLOC(currentString, neededSize);
                    currentSize = neededSize;
                }

                currentString[length++] = currentByte;
            }

            token.data = voxel_newString(context, length, currentString);

            VOXEL_FREE(currentString);

            #ifdef VOXEL_DEBUG
                VOXEL_LOG("[Token: string]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_CALL:
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
