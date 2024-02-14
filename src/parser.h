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