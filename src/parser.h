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
    VOXEL_COUNT tokenCount = 0;
    VOXEL_COUNT bytePosition = 0;

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

    for (VOXEL_COUNT i = 0; i < tokenCount; i++) {
        context->tokens[i] = currentTokenItem->token;
        nextTokenItem = currentTokenItem->nextTokenItem;

        VOXEL_FREE(currentTokenItem);

        currentTokenItem = nextTokenItem;
    }

    context->tokenCount = tokenCount;

    return VOXEL_OK;
}