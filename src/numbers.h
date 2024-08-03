voxel_Thing* voxel_newNumberInt(voxel_Context* context, voxel_Int value) {
    voxel_Number* number = (voxel_Number*)VOXEL_MALLOC(sizeof(voxel_Number)); VOXEL_TAG_MALLOC(voxel_Number);

    number->type = VOXEL_NUMBER_TYPE_INT;
    number->value.asInt = value;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NUMBER);

    thing->type = VOXEL_TYPE_NUMBER;
    thing->value = number;

    return thing;
}

voxel_Thing* voxel_newNumberFloat(voxel_Context* context, voxel_Float value) {
    voxel_Number* number = (voxel_Number*)VOXEL_MALLOC(sizeof(voxel_Number)); VOXEL_TAG_MALLOC(voxel_Number);

    number->type = VOXEL_NUMBER_TYPE_FLOAT;
    number->value.asFloat = value;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NUMBER);

    thing->type = VOXEL_TYPE_NUMBER;
    thing->value = number;

    return thing;
}

voxel_Int voxel_getNumberInt(voxel_Thing* thing) {
    if (thing->type != VOXEL_TYPE_NUMBER) {
        return 0;
    }

    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return (voxel_Int)number->value.asFloat;
    }
}

voxel_Float voxel_getNumberFloat(voxel_Thing* thing) {
    if (thing->type != VOXEL_TYPE_NUMBER) {
        return 0;
    }

    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return (voxel_Float)number->value.asInt;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return number->value.asFloat;
    }
}

VOXEL_ERRORABLE voxel_destroyNumber(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_NUMBER);

    VOXEL_FREE(thing->value); VOXEL_TAG_FREE(voxel_Number);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareNumbers(voxel_Thing* a, voxel_Thing* b) {
    voxel_Number* aNumber = (voxel_Number*)a->value;
    voxel_Number* bNumber = (voxel_Number*)b->value;

    if (aNumber->type == VOXEL_NUMBER_TYPE_INT && bNumber->type == VOXEL_NUMBER_TYPE_INT) {
        return aNumber->value.asInt == bNumber->value.asInt;
    }

    return voxel_getNumberFloat(a) == voxel_getNumberFloat(b);
}

voxel_Thing* voxel_copyNumber(voxel_Context* context, voxel_Thing* thing) {
    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return voxel_newNumberInt(context, number->value.asInt);

        case VOXEL_NUMBER_TYPE_FLOAT:
            return voxel_newNumberFloat(context, number->value.asFloat);
    }
}

VOXEL_ERRORABLE voxel_numberToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Float value = voxel_getNumberFloat(thing);
    voxel_Bool isNegative = VOXEL_FALSE;

    if (value < 0) {
        isNegative = VOXEL_TRUE;
        value *= -1;
    }

    #ifdef VOXEL_NAN
        if (value == VOXEL_NAN) {
            return VOXEL_OK_RET(voxel_newStringTerminated(context, "nan"));
        }
    #endif

    #ifdef VOXEL_INFINITY
        if (value == VOXEL_INFINITY) {
            return VOXEL_OK_RET(voxel_newStringTerminated(context, isNegative ? (voxel_Byte*)"-infinity" : (voxel_Byte*)"infinity"));
        }
    #endif

    voxel_Int exponent = 0;
    voxel_Thing* string = voxel_newString(context, 0, VOXEL_NULL);
    voxel_Count precisionLeft = VOXEL_MAX_PRECISION;

    if (value > 0) {
        if (value < voxel_maths_power(10, -VOXEL_MAX_PRECISION + 1)) {
            while (value < 1 - voxel_maths_power(10, -VOXEL_MAX_PRECISION)) {
                value *= 10;
                exponent--;
            }
        }

        if (value > voxel_maths_power(10, VOXEL_MAX_PRECISION - 1)) {
            while (value > 10 + voxel_maths_power(10, -VOXEL_MAX_PRECISION)) {
                value /= 10;
                exponent++;
            }
        }
    }

    value = voxel_maths_roundToPrecision(value, VOXEL_MAX_PRECISION);

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

            VOXEL_MUST(voxel_appendByteToString(context, string, (voxel_Byte)('0' + digit)));

            value -= digit;
            precisionLeft--;
        }
    }

    if (trailingZeroes > 0) {
        if (!anyDigitsInFractionalPart) {
            trailingZeroes++;
        }

        VOXEL_MUST(voxel_cutStringEnd(context, string, voxel_getStringSize(string) - trailingZeroes));
    }

    if (exponent != 0) {
        VOXEL_MUST(voxel_appendByteToString(context, string, 'E'));

        if (exponent > 0) {
            voxel_appendByteToString(context, string, '+');
        }

        voxel_Thing* exponentNumber = voxel_newNumberInt(context, exponent);
        VOXEL_ERRORABLE exponentString = voxel_numberToString(context, exponentNumber); VOXEL_MUST(exponentString);

        voxel_appendToString(context, string, (voxel_Thing*)exponentString.value);

        voxel_unreferenceThing(context, exponentNumber);
        voxel_unreferenceThing(context, (voxel_Thing*)exponentString.value);
    }

    return VOXEL_OK_RET(string);
}

VOXEL_ERRORABLE voxel_numberToBaseString(voxel_Context* context, voxel_Thing* thing, voxel_Count base, voxel_Count minSize) {
    const voxel_Byte* NUMERALS = "0123456789ABCDEF";

    voxel_Int value = voxel_getNumberInt(thing);
    voxel_Thing* string = voxel_newString(context, 0, VOXEL_NULL);
    voxel_Bool isNegative = VOXEL_FALSE;

    if (base < 2 || base > 16) {
        VOXEL_THROW(VOXEL_ERROR_INVALID_ARG);
    }

    if (value < 0) {
        isNegative = VOXEL_TRUE;
        value *= -1;
    }

    do {
        VOXEL_MUST(voxel_appendByteToString(context, string, NUMERALS[value % base]));

        value /= base;
    } while (value > 0);

    voxel_Thing* fill = voxel_newStringTerminated(context, "0");

    VOXEL_MUST(voxel_padStringEnd(context, string, minSize, fill));

    voxel_unreferenceThing(context, fill);

    if (isNegative) {
        VOXEL_MUST(voxel_appendByteToString(context, string, '-'));
    }

    voxel_reverseString(context, string);

    return VOXEL_OK_RET(string);
}

VOXEL_ERRORABLE voxel_numberToByte(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newByte(context, (voxel_Byte)voxel_getNumberInt(thing)));
}

voxel_Bool voxel_numberIsTruthy(voxel_Thing* thing) {
    voxel_Number* number = (voxel_Number*)thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return number->value.asInt != 0;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return number->value.asFloat != 0.0;
    }
}

voxel_Bool voxel_isNan(voxel_Thing* thing) {
    voxel_Float value = voxel_getNumberFloat(thing);

    return value != value;
}