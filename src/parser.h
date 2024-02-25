VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Position* position, voxel_Count bytesToRead) {
    if ((*position) + bytesToRead > context->codeLength) {
        VOXEL_THROW(VOXEL_ERROR_TOKENISATION_END);
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_nextToken(voxel_Context* context, voxel_Position* position) {
    VOXEL_ASSERT(context->code, VOXEL_ERROR_NO_CODE);
    
    voxel_Token token;

    VOXEL_MUST(voxel_safeToRead(context, position, 1));

    voxel_Bool shouldCreateToken = VOXEL_TRUE;
    voxel_Byte tokenType = context->code[(*position)++];

    switch (tokenType) {
        case VOXEL_TOKEN_TYPE_NULL:
            token.data = voxel_newNull(context);

            VOXEL_DEBUG_LOG("[Token: null]\n");

            break;

        case VOXEL_TOKEN_TYPE_BOOLEAN_TRUE:
        case VOXEL_TOKEN_TYPE_BOOLEAN_FALSE:
            token.data = voxel_newBoolean(context, tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE);

            VOXEL_DEBUG_LOG("[Token: bool (");
            VOXEL_DEBUG_LOG(tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE ? "true" : "false");
            VOXEL_DEBUG_LOG(")]\n");

            break;

        case VOXEL_TOKEN_TYPE_BYTE:
            VOXEL_MUST(voxel_safeToRead(context, position, 1));

            token.data = voxel_newByte(context, context->code[(*position)++]);

            VOXEL_DEBUG_LOG("[Token: byte]\n");

            break;

        case VOXEL_TOKEN_TYPE_NUMBER_INT_8:
        case VOXEL_TOKEN_TYPE_NUMBER_INT_16:
        case VOXEL_TOKEN_TYPE_NUMBER_INT_32:
            VOXEL_MUST(voxel_safeToRead(context, position, 1));

            voxel_Int numberIntValue = context->code[(*position)++];

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_16 || tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, position, 1));

                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++];
            }

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, position, 2));

                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++];
                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++];
            }

            token.data = voxel_newNumberInt(context, numberIntValue);

            VOXEL_DEBUG_LOG("[Token: num (int)]\n");

            break;

        case VOXEL_TOKEN_TYPE_NUMBER_FLOAT:
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_Float numberFloatValue;

            voxel_copy(&(context->code[(*position)]), (char*)&numberFloatValue, 4);

            (*position) += 4;
            token.data = voxel_newNumberFloat(context, numberFloatValue);

            VOXEL_DEBUG_LOG("[Token: num (float)]\n");

            break;

        case VOXEL_TOKEN_TYPE_BUFFER:
        case VOXEL_TOKEN_TYPE_BUFFER_EMPTY:
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_UInt32 bufferSize = 0;

            for (voxel_Count i = 0; i < 4; i++) {
                bufferSize <<= 8;
                bufferSize |= context->code[(*position)++];
            }

            if (tokenType == VOXEL_TOKEN_TYPE_BUFFER_EMPTY) {
                token.data = voxel_newBuffer(context, bufferSize, VOXEL_NULL);

                VOXEL_DEBUG_LOG("[Token: buffer (empty)]\n");

                break;
            }

            VOXEL_MUST(voxel_safeToRead(context, position, bufferSize));

            token.data = voxel_newBuffer(context, bufferSize, &(context->code[(*position)]));
            (*position) += bufferSize;

            VOXEL_DEBUG_LOG("[Token: buffer (declared)]\n");

            break;

        case VOXEL_TOKEN_TYPE_STRING:
            voxel_Byte currentByte = '\0';
            voxel_Byte* currentString = NULL;
            voxel_Count currentSize = 0;
            voxel_Count stringSize = 0;

            while (VOXEL_TRUE) {
                VOXEL_MUST(voxel_safeToRead(context, position, 1));

                currentByte = context->code[(*position)++];

                if (currentByte == '\0') {
                    break;
                }

                voxel_Count neededSize = (((stringSize / VOXEL_STRING_BLOCK_SIZE) + 1) * VOXEL_STRING_BLOCK_SIZE);

                if (currentString == NULL) {
                    currentString = VOXEL_MALLOC(neededSize);
                    currentSize = neededSize;
                } else if (neededSize > currentSize) {
                    currentString = VOXEL_REALLOC(currentString, neededSize);
                    currentSize = neededSize;
                }

                currentString[stringSize++] = currentByte;
            }

            token.data = voxel_newString(context, stringSize, currentString);

            VOXEL_FREE(currentString);

            VOXEL_DEBUG_LOG("[Token: string]\n");

            break;

        case VOXEL_TOKEN_TYPE_CALL:
        case VOXEL_TOKEN_TYPE_RETURN:
        case VOXEL_TOKEN_TYPE_GET:
        case VOXEL_TOKEN_TYPE_SET:
            break;

        case '\0':
            VOXEL_DEBUG_LOG("[Last byte]\n");

            return VOXEL_OK_RET(VOXEL_NULL);

        default:
            VOXEL_THROW(VOXEL_ERROR_TOKENISATION_BYTE);
    }

    token.type = tokenType;

    voxel_Token* tokenPtr = VOXEL_MALLOC(sizeof(token));

    VOXEL_INTO_PTR(token, tokenPtr);

    return VOXEL_OK_RET(tokenPtr);
}