VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Position* position, voxel_Count bytesToRead) {
    if ((*position) + bytesToRead > context->codeSize) {
        VOXEL_THROW(VOXEL_ERROR_TOKENISATION_END);
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_nextToken(voxel_Executor* executor, voxel_Position* position) {
    voxel_Context* context = executor->context;

    VOXEL_ASSERT(context->code, VOXEL_ERROR_NO_CODE);

    voxel_Token token;
    voxel_Byte tokenType;

    switch (executor->tokenisationState) {
        case VOXEL_STATE_SYSTEM_CALL_GET:
            tokenType = VOXEL_TOKEN_TYPE_GET;
            executor->tokenisationState = VOXEL_STATE_SYSTEM_CALL_CALL;
            break;

        case VOXEL_STATE_SYSTEM_CALL_CALL:
            tokenType = VOXEL_TOKEN_TYPE_CALL;
            executor->tokenisationState = VOXEL_STATE_NONE;
            break;

        default:
            VOXEL_MUST(voxel_safeToRead(context, position, 1));

            tokenType = context->code[(*position)++];

            break;
    }

    voxel_Bool shouldCreateToken = VOXEL_TRUE;

    switch (tokenType) {
        case VOXEL_TOKEN_TYPE_NULL:
            token.data = voxel_newNull(context);

            VOXEL_DEBUG_LOG("[Token: null]\n");

            break;

        case VOXEL_TOKEN_TYPE_BOOLEAN_TRUE:
        case VOXEL_TOKEN_TYPE_BOOLEAN_FALSE:
            token.data = voxel_newBoolean(context, tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE);

            VOXEL_DEBUG_LOG("[Token: bool (");
            VOXEL_DEBUG_LOG(tokenType == VOXEL_TOKEN_TYPE_BOOLEAN_TRUE ? (voxel_Byte*)"true" : (voxel_Byte*)"false");
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
        {
            VOXEL_MUST(voxel_safeToRead(context, position, 1));

            voxel_Int numberIntValue = context->code[(*position)++] & 0xFF;

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_16 || tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, position, 1));

                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++] & 0xFF;
            }

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32) {
                VOXEL_MUST(voxel_safeToRead(context, position, 2));

                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++] & 0xFF;
                numberIntValue <<= 8; numberIntValue |= context->code[(*position)++] & 0xFF;
            }

            if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_8 && numberIntValue & 0x80) {
                numberIntValue -= 0x100;
            } else if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_16 && numberIntValue & 0x8000) {
                numberIntValue -= 0x10000;
            } else if (tokenType == VOXEL_TOKEN_TYPE_NUMBER_INT_32 && numberIntValue & 0x80000000) {
                numberIntValue -= 0x100000000;
            }

            token.data = voxel_newNumberInt(context, numberIntValue);

            VOXEL_DEBUG_LOG("[Token: num (int)]\n");

            break;
        }

        case VOXEL_TOKEN_TYPE_NUMBER_FLOAT:
        {
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_Float numberFloatValue;

            voxel_copy(&(context->code[(*position)]), (char*)&numberFloatValue, 4);

            (*position) += 4;
            token.data = voxel_newNumberFloat(context, numberFloatValue);

            VOXEL_DEBUG_LOG("[Token: num (float)]\n");

            break;
        }

        case VOXEL_TOKEN_TYPE_BUFFER:
        case VOXEL_TOKEN_TYPE_BUFFER_EMPTY:
        {
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_UInt32 bufferSize = 0;

            for (voxel_Count i = 0; i < 4; i++) {
                bufferSize <<= 8;
                bufferSize |= context->code[(*position)++] & 0xFF;
            }

            if (tokenType == VOXEL_TOKEN_TYPE_BUFFER_EMPTY) {
                #ifdef VOXEL_MAX_BUFFER_INIT_SIZE
                    if (bufferSize > VOXEL_MAX_BUFFER_INIT_SIZE) {
                        bufferSize = VOXEL_MAX_BUFFER_INIT_SIZE;
                    }
                #endif

                token.data = voxel_newBuffer(context, bufferSize, VOXEL_NULL);

                VOXEL_DEBUG_LOG("[Token: buffer (empty)]\n");

                break;
            }

            VOXEL_MUST(voxel_safeToRead(context, position, bufferSize));

            token.data = voxel_newBuffer(context, bufferSize, &(context->code[(*position)]));
            (*position) += bufferSize;

            VOXEL_DEBUG_LOG("[Token: buffer (declared)]\n");

            break;
        }

        case VOXEL_TOKEN_TYPE_STRING:
        case VOXEL_TOKEN_TYPE_SYSTEM_CALL:
        {
            voxel_Byte currentByte = '\0';
            voxel_Byte* currentString = VOXEL_NULL;
            voxel_Count currentSize = 0;
            voxel_Count stringSize = 0;
            voxel_Bool needToAddPrefix = tokenType == VOXEL_TOKEN_TYPE_SYSTEM_CALL;

            while (VOXEL_TRUE) {
                VOXEL_MUST(voxel_safeToRead(context, position, 1));

                currentByte = needToAddPrefix ? '.' : context->code[(*position)++];
                needToAddPrefix = VOXEL_FALSE;

                if (currentByte == '\0') {
                    break;
                }

                voxel_Count neededSize = (((stringSize / VOXEL_STRING_BLOCK_SIZE) + 1) * VOXEL_STRING_BLOCK_SIZE);

                if (currentString == VOXEL_NULL) {
                    currentString = (voxel_Byte*)VOXEL_MALLOC(neededSize); VOXEL_TAG_MALLOC_SIZE("voxel_Byte[]", neededSize);
                    currentSize = neededSize;
                } else if (neededSize > currentSize) {
                    currentString = (voxel_Byte*)VOXEL_REALLOC(currentString, neededSize); VOXEL_TAG_REALLOC("voxel_Byte[]", currentSize, neededSize);
                    currentSize = neededSize;
                }

                currentString[stringSize++] = currentByte;
            }

            token.data = voxel_newString(context, stringSize, currentString);

            VOXEL_FREE(currentString); VOXEL_TAG_FREE_SIZE("voxel_Byte[]", currentSize);

            VOXEL_DEBUG_LOG("[Token: string]\n");

            if (tokenType == VOXEL_TOKEN_TYPE_SYSTEM_CALL) {
                executor->tokenisationState = VOXEL_STATE_SYSTEM_CALL_GET;
            }

            break;
        }

        case VOXEL_TOKEN_TYPE_CALL:
        case VOXEL_TOKEN_TYPE_RETURN:
        case VOXEL_TOKEN_TYPE_THROW:
        case VOXEL_TOKEN_TYPE_SET_HANDLER:
        case VOXEL_TOKEN_TYPE_CLEAR_HANDLER:
        case VOXEL_TOKEN_TYPE_GET:
        case VOXEL_TOKEN_TYPE_SET:
        case VOXEL_TOKEN_TYPE_VAR:
        case VOXEL_TOKEN_TYPE_POP:
        case VOXEL_TOKEN_TYPE_DUPE:
        case VOXEL_TOKEN_TYPE_OVER:
        case VOXEL_TOKEN_TYPE_SWAP:
        case VOXEL_TOKEN_TYPE_COPY:
        case VOXEL_TOKEN_TYPE_POS_REF_HERE:
        case VOXEL_TOKEN_TYPE_JUMP:
        case VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY:
        case VOXEL_TOKEN_TYPE_IDENTICAL:
        case VOXEL_TOKEN_TYPE_EQUAL:
        case VOXEL_TOKEN_TYPE_LESS_THAN:
        case VOXEL_TOKEN_TYPE_GREATER_THAN:
        case VOXEL_TOKEN_TYPE_NOT:
        case VOXEL_TOKEN_TYPE_AND:
        case VOXEL_TOKEN_TYPE_OR:
        case VOXEL_TOKEN_TYPE_ENUM_LOOKUP_REGISTER:
            #ifdef VOXEL_DEBUG
                voxel_Byte charString[2] = {(voxel_Byte)tokenType, '\0'};

                VOXEL_DEBUG_LOG("[Non-thing token ");
                VOXEL_DEBUG_LOG(charString);
                VOXEL_DEBUG_LOG("]\n");
            #endif

            break;

        case VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE:
        case VOXEL_TOKEN_TYPE_POS_REF_BACKWARD:
        case VOXEL_TOKEN_TYPE_POS_REF_FORWARD:
        {
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_UInt32 stepSize = 0;

            for (voxel_Count i = 0; i < 4; i++) {
                stepSize <<= 8;
                stepSize |= context->code[(*position)++] & 0xFF;
            }

            token.data = (void*)(voxel_IntPtr)stepSize;

            VOXEL_DEBUG_LOG("[Token: position reference]\n");

            break;
        }

        case '\0':
            VOXEL_DEBUG_LOG("[Last byte]\n");

            return VOXEL_OK_RET(VOXEL_NULL);

        default:
            VOXEL_THROW(VOXEL_ERROR_TOKENISATION_BYTE);
    }

    token.type = (voxel_TokenType)tokenType;

    voxel_Token* tokenPtr = (voxel_Token*)VOXEL_MALLOC(sizeof(token)); VOXEL_TAG_MALLOC(voxel_Token);

    VOXEL_INTO_PTR(token, tokenPtr);

    return VOXEL_OK_RET(tokenPtr);
}