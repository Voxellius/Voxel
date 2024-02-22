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

#ifdef VOXEL_DEBUG
    #define VOXEL_DEBUG_LOG VOXEL_LOG
#else
    void VOXEL_DEBUG_LOG(char* text) {}
#endif

void voxel_copy(voxel_Byte* source, voxel_Byte* destination, voxel_Count size) {
    for (voxel_Count i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

voxel_Bool voxel_compare(voxel_Byte* a, voxel_Byte* b, voxel_Count aSize, voxel_Count bSize) {
    if (aSize != bSize) {
        return VOXEL_FALSE;
    }

    for (voxel_Count i = 0; i < aSize; i++) {
        if (a[i] != b[i]) {
            return VOXEL_FALSE;
        }
    }

    return VOXEL_TRUE;
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
#define VOXEL_ERROR_THING_LOCKED -6
#define VOXEL_ERROR_NOT_A_MEMBER -7
#define VOXEL_ERROR_INVALID_ARGUMENT -8

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

        case VOXEL_ERROR_THING_LOCKED:
            return "Thing is locked (possibly as it belongs to a constant)";

        case VOXEL_ERROR_NOT_A_MEMBER:
            return "Not a member of an object";

        case VOXEL_ERROR_INVALID_ARGUMENT:
            return "Invalid argument";

        default:
            return "Unknown error";
    }
}

// src/maths.h

voxel_Float voxel_maths_power(voxel_Float base, voxel_Int power) {
    if (power == 0) {
        return 1;
    }

    if (power < 0) {
        base = 1 / base;
        power *= -1;
    }

    voxel_Float result = base;

    while (power > 1) {
        result *= base;
        power--;
    }

    return result;
}

voxel_Float voxel_maths_roundToPrecision(voxel_Float number, voxel_Count precision) {
    voxel_Bool isNegative = VOXEL_FALSE;

    if (number < 0) {
        isNegative = VOXEL_TRUE;
        number *= -1;
    }

    voxel_UInt integralPart = number;
    voxel_Count integralDigits = 0;

    while (integralPart > 0) {
        integralPart /= 10;
        integralDigits++;
    }

    precision -= integralDigits;

    if (precision < 0) {
        precision = 0;
    }

    voxel_UInt multiplier = voxel_maths_power(10, precision);

    number += 0.5 * voxel_maths_power(10, -precision);

    if (isNegative) {
        number *= -1;
    }

    return (voxel_Float)((voxel_Int)(number * multiplier)) / multiplier;
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
    VOXEL_TYPE_STRING,
    VOXEL_TYPE_OBJECT
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* value;
    voxel_Count referenceCount;
    voxel_Bool isLocked;
    struct voxel_Thing* previousTrackedThing;
    struct voxel_Thing* nextTrackedThing;
} voxel_Thing;

struct voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data);

void voxel_destroyNull(voxel_Thing* thing);
void voxel_destroyBoolean(voxel_Thing* thing);
void voxel_destroyByte(voxel_Thing* thing);
void voxel_destroyNumber(voxel_Thing* thing);
void voxel_destroyBuffer(voxel_Thing* thing);
void voxel_destroyString(voxel_Thing* thing);
void voxel_destroyObject(voxel_Context* context, voxel_Thing* thing);

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareNumbers(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b);

void voxel_lockObject(voxel_Thing* objectThing);

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_numberToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_bufferToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_stringToString(voxel_Context* context, voxel_Thing* thing);

voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = VOXEL_MALLOC(sizeof(voxel_Thing));

    thing->type = VOXEL_TYPE_NULL;
    thing->value = VOXEL_NULL;
    thing->referenceCount = 1;
    thing->isLocked = VOXEL_FALSE;
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

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_TRUE;
}

VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "null"));
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

voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, thing->value ? "true" : "false"));
}

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(long)value;
}

void voxel_destroyByte(voxel_Thing* thing) {
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

VOXEL_ERRORABLE voxel_destroyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: voxel_destroyNull(thing); return VOXEL_OK;
        case VOXEL_TYPE_BOOLEAN: voxel_destroyBoolean(thing); return VOXEL_OK;
        case VOXEL_TYPE_BYTE: voxel_destroyByte(thing); return VOXEL_OK;
        case VOXEL_TYPE_NUMBER: voxel_destroyNumber(thing); return VOXEL_OK;
        case VOXEL_TYPE_BUFFER: voxel_destroyBuffer(thing); return VOXEL_OK;
        case VOXEL_TYPE_STRING: voxel_destroyString(thing); return VOXEL_OK;
        case VOXEL_TYPE_OBJECT: voxel_destroyObject(context, thing); return VOXEL_OK;
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

    VOXEL_MUST(voxel_destroyThing(context, thing));
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

voxel_Bool voxel_compareThingTypes(voxel_Thing* a, voxel_Thing* b) {
    return a->type == b->type;
}

voxel_Bool voxel_compareThings(voxel_Thing* a, voxel_Thing* b) {
    if (!voxel_compareThingTypes(a, b)) {
        return VOXEL_FALSE;
    }

    switch (a->type) {
        case VOXEL_TYPE_NULL: return voxel_compareNulls(a, b);
        case VOXEL_TYPE_BOOLEAN: return voxel_compareBooleans(a, b);
        case VOXEL_TYPE_BYTE: return voxel_compareBytes(a, b);
        case VOXEL_TYPE_NUMBER: return voxel_compareNumbers(a, b);
        case VOXEL_TYPE_BUFFER: return voxel_compareBuffers(a, b);
        case VOXEL_TYPE_STRING: return voxel_compareStrings(a, b);
        // TODO: Compare objects
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning `VOXEL_FALSE` for now");

    return VOXEL_FALSE;
}

void voxel_lockThing(voxel_Thing* thing) {
    if (thing->isLocked) {
        return; // Prevents infinite recursive locking from happening for circular references
    }

    thing->isLocked = VOXEL_TRUE;

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT:
            voxel_lockObject(thing);
            break;
    }
}

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullToString(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToString(context, thing);
        // TODO: Implement others
    }
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

voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b) {
    voxel_Buffer* aBuffer = a->value;
    voxel_Buffer* bBuffer = b->value;

    return voxel_compare(aBuffer->value, bBuffer->value, aBuffer->size, bBuffer->size);
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
    thing->value = string;

    if (length > 0) {
        voxel_copy(data, string->value, length);
    }

    return thing;
}

voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data) {
    voxel_Count length = 0;

    while (data[length] != '\0') {
        length++;
    }

    return voxel_newString(context, length, data);
}

void voxel_destroyString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    VOXEL_FREE(string->value);
    VOXEL_FREE(string);
    VOXEL_FREE(thing);
}

voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    return voxel_compare(aString->value, bString->value, aString->length, bString->length);
}

voxel_Count voxel_getStringLength(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return string->length;
}

void voxel_logString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    for (voxel_Count i = 0; i < string->length; i++) {
        VOXEL_LOG_BYTE(string->value[i]);
    }
}

voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_String* resultString = VOXEL_MALLOC(sizeof(voxel_String));

    resultString->length = aString->length + bString->length;
    resultString->value = VOXEL_MALLOC(resultString->length);

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = resultString;

    voxel_Count position = 0;

    for (voxel_Count i = 0; i < aString->length; i++) {
        resultString->value[position++] = aString->value[i];
    }

    for (voxel_Count i = 0; i < bString->length; i++) {
        resultString->value[position++] = bString->value[i];
    }

    return thing;
}

VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ASSERT(!a->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_Count newLength = aString->length + bString->length;

    aString->value = VOXEL_REALLOC(aString->value, newLength);

    for (voxel_Count i = 0; i < bString->length; i++) {
        aString->value[aString->length + i] = bString->value[i];
    }

    aString->length = newLength;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_appendToStringTerminatedBytes(voxel_Context* context, voxel_Thing* a, voxel_Byte* b) {
    voxel_Thing* bThing = voxel_newStringTerminated(context, b);

    VOXEL_MUST(voxel_appendToString(context, a, bThing));
    VOXEL_MUST(voxel_unreferenceThing(context, bThing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_appendByteToString(voxel_Context* context, voxel_Thing* thing, voxel_Byte byte) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;

    string->length++;
    string->value = VOXEL_REALLOC(string->value, string->length);

    string->value[string->length - 1] = byte;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_cutString(voxel_Context* context, voxel_Thing* thing, voxel_Count length) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    VOXEL_ASSERT(length > 0, VOXEL_ERROR_INVALID_ARGUMENT);

    voxel_String* string = thing->value;

    if (string->length < length) {
        return VOXEL_OK;
    }

    string->length = length;
    string->value = VOXEL_REALLOC(string->value, string->length);

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;
    voxel_Byte sourceString[string->length];

    voxel_copy(string->value, sourceString, string->length);

    for (voxel_Count i = 0; i < string->length; i++) {
        string->value[string->length - 1 - i] = sourceString[i];
    }

    return VOXEL_OK;
}

// src/objects.h

typedef struct voxel_Object {
    voxel_Count length;
    struct voxel_ObjectItem* firstItem;
    struct voxel_ObjectItem* lastItem;
} voxel_Object;

typedef struct voxel_ObjectItem {
    voxel_Thing* key;
    voxel_Thing* value;
    struct voxel_ObjectItem* nextItem;
} voxel_ObjectItem;

voxel_Thing* voxel_newObject(voxel_Context* context) {
    voxel_Object* object = VOXEL_MALLOC(sizeof(voxel_Object));

    object->length = 0;
    object->firstItem = VOXEL_NULL;
    object->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context);

    thing->type = VOXEL_TYPE_OBJECT;
    thing->value = object;

    return thing;
}

voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* thing, voxel_Thing* key) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            return VOXEL_NULL;
        }

        if (voxel_compareThings(currentItem->key, key)) {
            return currentItem;
        }

        currentItem = currentItem->nextItem;
    }
}

VOXEL_ERRORABLE voxel_setObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_Object* object = thing->value;
    voxel_ObjectItem* objectItem = voxel_getObjectItem(thing, key);

    if (objectItem) {
        voxel_unreferenceThing(context, objectItem->value);

        objectItem->value = value;
        value->referenceCount++;

        return VOXEL_OK;
    }

    voxel_lockThing(key);

    objectItem = VOXEL_MALLOC(sizeof(voxel_ObjectItem));

    objectItem->key = key;
    key->referenceCount++;

    objectItem->value = value;
    value->referenceCount++;

    objectItem->nextItem = VOXEL_NULL;

    if (!object->firstItem) {
        object->firstItem = objectItem;
    }

    if (object->lastItem) {
        object->lastItem->nextItem = objectItem;
    }

    object->length++;
    object->lastItem = objectItem;

    return VOXEL_OK;
}

VOXEL_ERRORABLE removeObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* previousItem = VOXEL_NULL;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            VOXEL_THROW(VOXEL_ERROR_THING_LOCKED);
        }

        if (voxel_compareThings(currentItem->key, key)) {
            if (currentItem == object->firstItem) {
                object->firstItem = currentItem->nextItem;
            } else {
                previousItem->nextItem = currentItem->nextItem;
            }

            if (currentItem == object->lastItem) {
                object->lastItem = previousItem;
            }

            voxel_unreferenceThing(context, currentItem->key);
            voxel_unreferenceThing(context, currentItem->value);

            VOXEL_FREE(currentItem);

            object->length--;

            return VOXEL_OK;
        }

        previousItem = currentItem;
        currentItem = currentItem->nextItem;
    }
}

void voxel_destroyObject(voxel_Context* context, voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* nextItem;

    while (currentItem) {
        voxel_unreferenceThing(context, currentItem->key);
        voxel_unreferenceThing(context, currentItem->value);

        nextItem = currentItem->nextItem;

        VOXEL_FREE(currentItem);

        currentItem = nextItem;
    }

    VOXEL_FREE(object);
    VOXEL_FREE(thing);
}

void voxel_lockObject(voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (currentItem) {
        voxel_lockThing(currentItem->value);

        currentItem = currentItem->nextItem;
    }
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
            VOXEL_MUST(voxel_safeToRead(context, 1));

            token.data = voxel_newByte(context, context->code[context->currentPosition++]);

            VOXEL_DEBUG_LOG("[Token: byte]\n");

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

            VOXEL_DEBUG_LOG("[Token: num (int)]\n");

            break;

        case VOXEL_TOKEN_TYPE_NUMBER_FLOAT:
            VOXEL_MUST(voxel_safeToRead(context, 4));

            voxel_Float numberFloatValue;

            voxel_copy(&(context->code[context->currentPosition]), (char*)&numberFloatValue, 4);

            context->currentPosition += 4;
            token.data = voxel_newNumberFloat(context, numberFloatValue);

            VOXEL_DEBUG_LOG("[Token: num (float)]\n");

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

                VOXEL_DEBUG_LOG("[Token: buffer (empty)]\n");

                break;
            }

            VOXEL_MUST(voxel_safeToRead(context, size));

            token.data = voxel_newBuffer(context, size, &(context->code[context->currentPosition]));
            context->currentPosition += size;

            VOXEL_DEBUG_LOG("[Token: buffer (declared)]\n");

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

            VOXEL_DEBUG_LOG("[Token: string]\n");

            break;

        case VOXEL_TOKEN_TYPE_CALL:
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

// src/voxel.h

void voxel_test() {
    VOXEL_LOG("Hello from Voxel!\n");
}

#endif
