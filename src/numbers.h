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

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count length, voxel_Byte* data);
voxel_Count voxel_getStringLength(voxel_Thing* thing);
VOXEL_ERRORABLE voxel_appendByteToString(voxel_Context* context, voxel_Thing* thing, voxel_Byte byte);
VOXEL_ERRORABLE voxel_cutString(voxel_Context* context, voxel_Thing* thing, voxel_Count length);
VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing);

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

voxel_Bool voxel_compareNumbers(voxel_Thing* a, voxel_Thing* b) {
    voxel_Number* aNumber = a->value;
    voxel_Number* bNumber = b->value;

    if (aNumber->type == VOXEL_NUMBER_TYPE_INT && bNumber->type == VOXEL_NUMBER_TYPE_INT) {
        return aNumber->value.asInt == bNumber->value.asInt;
    }

    return voxel_getNumberFloat(a) == voxel_getNumberFloat(b);
}

VOXEL_ERRORABLE voxel_numberToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Float value = voxel_maths_roundToPrecision(voxel_getNumberFloat(thing), VOXEL_MAX_PRECISION);
    voxel_Thing* string = voxel_newString(context, 0, VOXEL_NULL);
    voxel_Bool isNegative = VOXEL_FALSE;
    voxel_Count precisionLeft = VOXEL_MAX_PRECISION;

    if (value < 0) {
        isNegative = VOXEL_TRUE;
        value *= -1;
    }

    voxel_UInt integralPart = value;

    value += 0.1 * voxel_maths_power(10, -precisionLeft);
    value -= integralPart; // Now fractional part

    do {
        VOXEL_MUST(voxel_appendByteToString(context, string, (voxel_Byte)(48 + integralPart % 10)));

        integralPart /= 10;
        precisionLeft--;
    } while (integralPart > 0);

    if (isNegative) {
        VOXEL_MUST(voxel_appendByteToString(context, string, '-'));
    }

    voxel_reverseString(context, string);

    voxel_Count trailingZeroes = 0;
    voxel_Bool anyDigitsInFractionalPart = VOXEL_FALSE;

    if (value > 0 && precisionLeft > 0) {
        VOXEL_MUST(voxel_appendByteToString(context, string, '.'));

        while (value > 0 && precisionLeft > 0) {
            value *= 10;

            voxel_Byte digit = value;

            if (digit == 0) {
                trailingZeroes++;
            } else {
                trailingZeroes = 0;
                anyDigitsInFractionalPart = VOXEL_TRUE;
            }

            VOXEL_MUST(voxel_appendByteToString(context, string, (voxel_Byte)(48 + digit)));

            value -= digit;
            precisionLeft--;
        }
    }

    if (trailingZeroes > 0) {
        if (!anyDigitsInFractionalPart) {
            trailingZeroes++;
        }

        voxel_cutString(context, string, voxel_getStringLength(string) - trailingZeroes);
    }

    return VOXEL_OK_RET(string);
}