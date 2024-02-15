typedef enum voxel_TokenType {
    VOXEL_TOKEN_TYPE_NULL = 'n',
    VOXEL_TOKEN_TYPE_FALSE = 'f',
    VOXEL_TOKEN_TYPE_TRUE = 't',
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
                } else {
                    currentString = VOXEL_REALLOC(currentString, neededSize);
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