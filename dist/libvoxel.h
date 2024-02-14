#ifndef LIBVOXEL_H_
#define LIBVOXEL_H_

// src/common.h

typedef VOXEL_BOOL voxel_Bool;
typedef VOXEL_COUNT voxel_Count;

#define VOXEL_TRUE 1
#define VOXEL_FALSE 0

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
#define VOXEL_ERROR_TOKENISATION -2

const char* voxel_lookupError(VOXEL_ERROR_CODE error) {
    switch (error) {
        case VOXEL_ERROR_NO_CODE:
            return "No code loaded";

        case VOXEL_ERROR_TOKENISATION:
            return "Error encountered when tokenising";

        default:
            return "Unknown error";
    }
}

// src/context.h

typedef struct voxel_Context {
    char* code;
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
    VOXEL_TYPE_NULL = 0,
    VOXEL_TYPE_POSITION = 1,
    VOXEL_TYPE_BYTE = 2,
    VOXEL_TYPE_NATIVE_FUNCTION = 3
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* valuePtr;
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
            VOXEL_FREE(thing->valuePtr);
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
    thing->valuePtr = NULL;
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

// src/parser.h

typedef enum voxel_TokenType {
    VOXEL_TOKEN_TYPE_FUNCTION_CALL,
    VOXEL_TOKEN_TYPE_THING
} voxel_TokenType;

typedef struct voxel_Token {
    voxel_TokenType type;
    void* data;
} voxel_Token;

typedef struct voxel_TokenItem {
    voxel_Token token;
    struct voxel_TokenItem* nextTokenItem;
} voxel_TokenItem;

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
        voxel_Bool shouldCreateToken = VOXEL_TRUE;

        switch (context->code[bytePosition]) {
            case '\0':
                shouldCreateToken = VOXEL_FALSE;
                break;

            case 'n':
                tokenToAdd.type = VOXEL_TOKEN_TYPE_THING;
                tokenToAdd.data = voxel_newNull(context);
                break;

            default:
                VOXEL_THROW(VOXEL_ERROR_TOKENISATION);
        }

        bytePosition++;

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
