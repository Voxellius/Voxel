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
typedef VOXEL_INTPTR voxel_IntPtr;

#define VOXEL_TRUE 1
#define VOXEL_FALSE 0
#define VOXEL_NULL 0

#define VOXEL_NOOP do {} while (0)

#define VOXEL_INTO_PTR(data, pointer) voxel_copy((voxel_Byte*)&(data), (voxel_Byte*)pointer, sizeof(data))

#ifdef VOXEL_DEBUG
    #define VOXEL_DEBUG_LOG VOXEL_LOG
#else
    #define VOXEL_DEBUG_LOG(text) VOXEL_NOOP
#endif

#ifdef VOXEL_DEBUG_THINGS
    #include <stdio.h>

    voxel_Count _voxel_thingCount = 0;

    #define VOXEL_TAG_NEW_THING(type) printf("[New thing %s = %d]\n", #type, ++_voxel_thingCount);
    #define VOXEL_TAG_DESTROY_THING(type) printf("[Destroy thing %s = %d]\n", #type, --_voxel_thingCount);
#else
    #define VOXEL_TAG_NEW_THING(type) VOXEL_NOOP
    #define VOXEL_TAG_DESTROY_THING(type) VOXEL_NOOP
#endif

#ifdef VOXEL_DEBUG_MEMORY
    #include <stdio.h>

    voxel_Count _voxel_memoryUsage = 0;

    #define VOXEL_TAG_MALLOC(type) printf("[Allocate memory for %s, +%d = %d]\n", #type, sizeof(type), _voxel_memoryUsage += sizeof(type))
    #define VOXEL_TAG_MALLOC_SIZE(name, size) printf("[Allocate memory for %s, +%d = %d]\n", name, size, _voxel_memoryUsage += size)
    #define VOXEL_TAG_FREE(type) printf("[Free memory for %s, -%d = %d]\n", #type, sizeof(type), _voxel_memoryUsage -= sizeof(type))
    #define VOXEL_TAG_FREE_SIZE(name, size) printf("[Free memory for %s, -%d = %d]\n", name, size, _voxel_memoryUsage -= size)
    #define VOXEL_TAG_REALLOC(name, oldSize, newSize) printf("[Reallocate memory for %s, -%d +%d = %d]\n", name, oldSize, newSize, _voxel_memoryUsage += newSize - oldSize)
#else
    #define VOXEL_TAG_MALLOC(type) VOXEL_NOOP
    #define VOXEL_TAG_MALLOC_SIZE(name, size) VOXEL_NOOP
    #define VOXEL_TAG_FREE(type) VOXEL_NOOP
    #define VOXEL_TAG_FREE_SIZE(name, size) VOXEL_NOOP
    #define VOXEL_TAG_REALLOC(name, oldSize, newSize) VOXEL_NOOP
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

#define VOXEL_OK_CODE 0x00
#define VOXEL_ERROR_NOT_INITIALISED -0x10
#define VOXEL_ERROR_NO_CODE -0x11
#define VOXEL_ERROR_INVALID_MAGIC -0x12
#define VOXEL_ERROR_TOKENISATION_BYTE -0x13
#define VOXEL_ERROR_TOKENISATION_END -0x14
#define VOXEL_ERROR_TYPE_MISMATCH -0x20
#define VOXEL_ERROR_NOT_IMPLEMENTED -0x21
#define VOXEL_ERROR_INVALID_ARG -0x22
#define VOXEL_ERROR_THING_LOCKED -0x30
#define VOXEL_ERROR_CANNOT_CONVERT_THING -0x31
#define VOXEL_ERROR_NOT_A_MEMBER -0x32
#define VOXEL_ERROR_CANNOT_CALL_THING -0x33
#define VOXEL_ERROR_CANNOT_JUMP_TO_THING -0x34
#define VOXEL_ERROR_INVALID_BUILTIN -0x35
#define VOXEL_ERROR_MISSING_ARG -0x40
#define VOXEL_ERROR_UNHANDLED_EXCEPTION -0x50

#define VOXEL_OK (voxel_Result) {.errorCode = VOXEL_OK_CODE, .value = VOXEL_NULL}
#define VOXEL_OK_RET(result) (voxel_Result) {.errorCode = VOXEL_OK_CODE, .value = (result)}

const voxel_Byte* voxel_lookupError(voxel_ErrorCode error) {
    switch (error) {
        case VOXEL_ERROR_NO_CODE:
            return "No code loaded";

        case VOXEL_ERROR_NOT_INITIALISED:
            return "Context not initialised yet";

        case VOXEL_ERROR_INVALID_MAGIC:
            return "Code has invalid magic";

        case VOXEL_ERROR_TOKENISATION_BYTE:
            return "Unknown byte when tokenising";

        case VOXEL_ERROR_TOKENISATION_END:
            return "Unexpectedly reached end when tokenising";

        case VOXEL_ERROR_TYPE_MISMATCH:
            return "Type mismatch";

        case VOXEL_ERROR_NOT_IMPLEMENTED:
            return "Not implemented";

        case VOXEL_ERROR_INVALID_ARG:
            return "Invalid argument";

        case VOXEL_ERROR_THING_LOCKED:
            return "Thing is locked (possibly as it belongs to a constant)";

        case VOXEL_ERROR_CANNOT_CONVERT_THING:
            return "Cannot convert thing into desired type or format";

        case VOXEL_ERROR_NOT_A_MEMBER:
            return "Not a member of an object";

        case VOXEL_ERROR_CANNOT_CALL_THING:
            return "Attempt to call a thing that is not a function";

        case VOXEL_ERROR_CANNOT_JUMP_TO_THING:
            return "Attempt to jump to a thing that is not a position reference";

        case VOXEL_ERROR_INVALID_BUILTIN:
            return "Invalid builtin function";

        case VOXEL_ERROR_MISSING_ARG:
            return "Missing a required argument on value stack";

        case VOXEL_ERROR_UNHANDLED_EXCEPTION:
            return "Unhandled exception";

        default:
            return "Unknown error";
    }
}

// src/declarations.h

typedef voxel_Count voxel_Position;

struct voxel_Executor;

typedef void (*voxel_Builtin)(struct voxel_Executor* executor);

typedef struct voxel_Result {
    voxel_ErrorCode errorCode;
    void* value;
} voxel_Result;

typedef enum {
    VOXEL_STATE_NONE,
    VOXEL_STATE_SYSTEM_CALL_GET,
    VOXEL_STATE_SYSTEM_CALL_CALL
} voxel_TokenisationState;

typedef struct voxel_Context {
    voxel_Bool isInitialised;
    char* code;
    voxel_Count codeLength;
    voxel_Builtin* builtins;
    voxel_Count builtinCount;
    voxel_TokenisationState tokenisationState;
    struct voxel_Scope* globalScope;
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
    struct voxel_Executor* firstExecutor;
    struct voxel_Executor* lastExecutor;
} voxel_Context;

typedef enum {
    VOXEL_TYPE_NULL,
    VOXEL_TYPE_BOOLEAN,
    VOXEL_TYPE_BYTE,
    VOXEL_TYPE_FUNCTION,
    VOXEL_TYPE_CLOSURE,
    VOXEL_TYPE_NUMBER,
    VOXEL_TYPE_BUFFER,
    VOXEL_TYPE_STRING,
    VOXEL_TYPE_OBJECT,
    VOXEL_TYPE_LIST
} voxel_DataType;

typedef struct voxel_Thing {
    voxel_DataType type;
    void* value;
    voxel_Count referenceCount;
    voxel_Bool isLocked;
    struct voxel_Thing* previousTrackedThing;
    struct voxel_Thing* nextTrackedThing;
} voxel_Thing;

typedef enum {
    VOXEL_FUNCTION_TYPE_BUILTIN,
    VOXEL_FUNCTION_TYPE_POS_REF
} voxel_FunctionType;

typedef enum {
    VOXEL_NUMBER_TYPE_INT,
    VOXEL_NUMBER_TYPE_FLOAT
} voxel_NumberType;

typedef struct voxel_Closure {
    voxel_Position position;
    voxel_Thing* environment;
} voxel_Closure;

typedef struct voxel_Number {
    voxel_NumberType type;
    union {
        voxel_Int asInt;
        voxel_Float asFloat;
    } value;
} voxel_Number;

typedef struct voxel_Buffer {
    voxel_Count size;
    voxel_Byte* value;
} voxel_Buffer;

typedef struct voxel_String {
    voxel_Count size;
    voxel_Byte* value;
} voxel_String;

typedef struct voxel_Object {
    voxel_Count length;
    struct voxel_ObjectItem* firstItem;
    struct voxel_ObjectItem* lastItem;
    voxel_Thing* prototypes;
} voxel_Object;

typedef struct voxel_ObjectItem {
    voxel_Thing* key;
    voxel_Thing* value;
    struct voxel_ObjectItemDescriptor* descriptor;
    struct voxel_ObjectItem* nextItem;
} voxel_ObjectItem;

typedef struct voxel_ObjectItemDescriptor {
    voxel_Thing* getterFunction;
    voxel_Thing* setterFunction;
} voxel_ObjectItemDescriptor;

typedef struct voxel_List {
    voxel_Count length;
    struct voxel_ListItem* firstItem;
    struct voxel_ListItem* lastItem;
} voxel_List;

typedef struct voxel_ListItem {
    voxel_Thing* value;
    struct voxel_ListItem* previousItem;
    struct voxel_ListItem* nextItem;
} voxel_ListItem;

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
    VOXEL_TOKEN_TYPE_STRING = '"',
    VOXEL_TOKEN_TYPE_CALL = '!',
    VOXEL_TOKEN_TYPE_SYSTEM_CALL = '.',
    VOXEL_TOKEN_TYPE_RETURN = '^',
    VOXEL_TOKEN_TYPE_THROW = 'T',
    VOXEL_TOKEN_TYPE_SET_HANDLER = 'H',
    VOXEL_TOKEN_TYPE_CLEAR_HANDLER = 'h',
    VOXEL_TOKEN_TYPE_GET = '?',
    VOXEL_TOKEN_TYPE_SET = ':',
    VOXEL_TOKEN_TYPE_VAR = 'v',
    VOXEL_TOKEN_TYPE_POP = 'p',
    VOXEL_TOKEN_TYPE_DUPE = 'd',
    VOXEL_TOKEN_TYPE_SWAP = 's',
    VOXEL_TOKEN_TYPE_POS_REF_HERE = '@',
    VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE = '#',
    VOXEL_TOKEN_TYPE_POS_REF_BACKWARD = '[',
    VOXEL_TOKEN_TYPE_POS_REF_FORWARD = ']',
    VOXEL_TOKEN_TYPE_JUMP = 'J',
    VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY = 'I',
    VOXEL_TOKEN_TYPE_EQUAL = '=',
    VOXEL_TOKEN_TYPE_LESS_THAN = '<',
    VOXEL_TOKEN_TYPE_GREATER_THAN = '>',
    VOXEL_TOKEN_TYPE_NOT = 'N',
    VOXEL_TOKEN_TYPE_AND = 'A',
    VOXEL_TOKEN_TYPE_OR = 'O'
} voxel_TokenType;

typedef struct voxel_Token {
    voxel_TokenType type;
    void* data;
} voxel_Token;

typedef struct voxel_Scope {
    voxel_Context* context;
    struct voxel_Scope* parentScope;
    voxel_Thing* things;
} voxel_Scope;

typedef struct voxel_Call {
    voxel_Position position;
    voxel_Bool canHandleExceptions;
    voxel_Position exceptionHandlerPosition;
} voxel_Call;

typedef struct voxel_Executor {
    voxel_Context* context;
    voxel_Scope* scope;
    voxel_Bool isRunning;
    voxel_Call* callStack;
    voxel_Count callStackHead;
    voxel_Count callStackSize;
    voxel_Thing* valueStack;
    struct voxel_Executor* previousExecutor;
    struct voxel_Executor* nextExecutor;
} voxel_Executor;

void voxel_copy(voxel_Byte* source, voxel_Byte* destination, voxel_Count size);
voxel_Bool voxel_compare(voxel_Byte* a, voxel_Byte* b, voxel_Count aSize, voxel_Count bSize);

const voxel_Byte* voxel_lookupError(voxel_ErrorCode error);

voxel_Float voxel_maths_power(voxel_Float base, voxel_Int power);
voxel_Float voxel_maths_roundToPrecision(voxel_Float number, voxel_Count precision);

voxel_Context* voxel_newContext();
VOXEL_ERRORABLE voxel_initContext(voxel_Context* context);
VOXEL_ERRORABLE voxel_stepContext(voxel_Context* context);
voxel_Bool voxel_anyContextsRunning(voxel_Context* context);
VOXEL_ERRORABLE voxel_defineBuiltin(voxel_Context* context, voxel_Byte* name, voxel_Builtin builtin);

voxel_Thing* voxel_newThing(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyThing(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_removeUnusedThings(voxel_Context* context);
voxel_Bool voxel_compareThingTypes(voxel_Thing* a, voxel_Thing* b);
voxel_Bool voxel_compareThings(voxel_Thing* a, voxel_Thing* b);
void voxel_lockThing(voxel_Thing* thing);
voxel_Thing* voxel_copyThing(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_thingToVxon(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_thingToByte(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_thingIsTruthy(voxel_Thing* thing);
VOXEL_ERRORABLE voxel_logThing(voxel_Context* context, voxel_Thing* thing);

voxel_Thing* voxel_newNull(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyNull(voxel_Thing* thing);
voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyNull(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_nullIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value);
VOXEL_ERRORABLE voxel_destroyBoolean(voxel_Thing* thing);
voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyBoolean(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_booleanToNumber(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_booleanIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value);
VOXEL_ERRORABLE voxel_destroyByte(voxel_Thing* thing);
voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyByte(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_byteToVxon(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_byteToNumber(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_byteIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newFunctionBuiltin(voxel_Context* context, voxel_Count builtinFunctionIndex);
voxel_Thing* voxel_newFunctionPosRef(voxel_Context* context, voxel_Position positionReference);
VOXEL_ERRORABLE voxel_destroyFunction(voxel_Thing* thing);
voxel_Bool voxel_compareFunctions(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyFunction(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_functionToString(voxel_Context* context, voxel_Thing* thing);
voxel_FunctionType voxel_getFunctionType(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_functionIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newClosure(voxel_Context* context, voxel_Position positionReference, voxel_Thing* environment);
VOXEL_ERRORABLE voxel_destroyClosure(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_compareClosures(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyClosure(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_closureToString(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_closureIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newNumberInt(voxel_Context* context, voxel_Int value);
voxel_Thing* voxel_newNumberFloat(voxel_Context* context, voxel_Float value);
voxel_Int voxel_getNumberInt(voxel_Thing* thing);
voxel_Float voxel_getNumberFloat(voxel_Thing* thing);
VOXEL_ERRORABLE voxel_destroyNumber(voxel_Thing* thing);
voxel_Bool voxel_compareNumbers(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyNumber(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_numberToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_numberToBaseString(voxel_Context* context, voxel_Thing* thing, voxel_Count base, voxel_Count minSize);
VOXEL_ERRORABLE voxel_numberToByte(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_numberIsTruthy(voxel_Thing* thing);

voxel_Thing* voxel_newBuffer(voxel_Context* context, voxel_Count size, voxel_Byte* data);
VOXEL_ERRORABLE voxel_destroyBuffer(voxel_Thing* thing);
voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyBuffer(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_bufferToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_bufferToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_bufferIsTruthy(voxel_Thing* thing);
voxel_Count voxel_getBufferSize(voxel_Thing* thing);

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count size, voxel_Byte* data);
voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data);
VOXEL_ERRORABLE voxel_destroyString(voxel_Thing* thing);
voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_stringToNumber(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_stringToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_stringIsTruthy(voxel_Thing* thing);
voxel_Count voxel_getStringSize(voxel_Thing* thing);
void voxel_logString(voxel_Thing* thing);
voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_appendToStringTerminatedBytes(voxel_Context* context, voxel_Thing* a, voxel_Byte* b);
VOXEL_ERRORABLE voxel_appendByteToString(voxel_Context* context, voxel_Thing* thing, voxel_Byte byte);
VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_cutStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count size);
VOXEL_ERRORABLE voxel_cutStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count size);
VOXEL_ERRORABLE voxel_padStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte);
VOXEL_ERRORABLE voxel_padStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte);

voxel_Thing* voxel_newObject(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyObject(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_compareObjects(voxel_Thing* a, voxel_Thing* b);
void voxel_lockObject(voxel_Thing* thing);
voxel_Thing* voxel_copyObject(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_objectToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_objectIsTruthy(voxel_Thing* thing);
voxel_ObjectItem* voxel_getPrototypedObjectItem(voxel_Thing* thing, voxel_Thing* key, voxel_Count traverseDepth);
voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* thing, voxel_Thing* key);
VOXEL_ERRORABLE voxel_setObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key, voxel_Thing* value);
VOXEL_ERRORABLE voxel_removeObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key);
voxel_ObjectItemDescriptor* voxel_ensureObjectItemDescriptor(voxel_Context* context, voxel_ObjectItem* objectItem);
voxel_Count voxel_getObjectLength(voxel_Thing* thing);
voxel_Thing* voxel_getObjectPrototypes(voxel_Context* context, voxel_Thing* thing);

voxel_Thing* voxel_newList(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyList(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_compareLists(voxel_Thing* a, voxel_Thing* b);
void voxel_lockList(voxel_Thing* thing);
voxel_Thing* voxel_copyList(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_listToString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_listToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_listIsTruthy(voxel_Thing* thing);
VOXEL_ERRORABLE voxel_getListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index);
VOXEL_ERRORABLE voxel_setListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value);
VOXEL_ERRORABLE voxel_removeListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index);
VOXEL_ERRORABLE voxel_pushOntoList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* value);
VOXEL_ERRORABLE voxel_popFromList(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_insertIntoList(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value);
voxel_Count voxel_getListLength(voxel_Thing* thing);
VOXEL_ERRORABLE voxel_joinList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* delimeter);

VOXEL_ERRORABLE voxel_notOperation(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_andOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_orOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_equalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_lessThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_greaterThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);

VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Position* position, voxel_Count bytesToRead);
VOXEL_ERRORABLE voxel_nextToken(voxel_Context* context, voxel_Position* position);

voxel_Scope* voxel_newScope(voxel_Context* context, voxel_Scope* parentScope);
VOXEL_ERRORABLE voxel_destroyScope(voxel_Scope* scope);
voxel_ObjectItem* voxel_getScopeItem(voxel_Scope* scope, voxel_Thing* key);
VOXEL_ERRORABLE voxel_setScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value);
VOXEL_ERRORABLE voxel_setLocalScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value);

voxel_Executor* voxel_newExecutor(voxel_Context* context);
voxel_Position* voxel_getExecutorPosition(voxel_Executor* executor);
VOXEL_ERRORABLE voxel_stepExecutor(voxel_Executor* executor);
void voxel_stepInExecutor(voxel_Executor* executor, voxel_Position position);
void voxel_stepOutExecutor(voxel_Executor* executor);
void voxel_setExceptionHandler(voxel_Executor* executor, voxel_Position exceptionHandlerPosition);
void voxel_clearExceptionHandler(voxel_Executor* executor);
VOXEL_ERRORABLE voxel_throwException(voxel_Executor* executor);

void voxel_push(voxel_Executor* executor, voxel_Thing* thing);
void voxel_pushNull(voxel_Executor* executor);
voxel_Thing* voxel_pop(voxel_Executor* executor);
void voxel_popVoid(voxel_Executor* executor);
voxel_Thing* voxel_popNumber(voxel_Executor* executor);
voxel_Int voxel_popNumberInt(voxel_Executor* executor);
voxel_Float voxel_popNumberFloat(voxel_Executor* executor);
voxel_Thing* voxel_popString(voxel_Executor* executor);
voxel_Thing* voxel_peek(voxel_Executor* executor, voxel_Int index);

void voxel_test();

// src/builtins/core/maths.h

#ifdef VOXEL_BUILTINS_CORE

#define _VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(name, operator) void name(voxel_Executor* executor) { \
        voxel_Int argCount = voxel_popNumberInt(executor); \
        voxel_Thing* b = voxel_popNumber(executor); \
        voxel_Thing* a = voxel_popNumber(executor); \
\
        if (!a || !b) { \
            voxel_pushNull(executor); \
\
            return; \
        } \
\
        voxel_push(executor, voxel_newNumberFloat(executor->context, voxel_getNumberFloat(a) operator voxel_getNumberFloat(b))); \
\
        voxel_unreferenceThing(executor->context, a); \
        voxel_unreferenceThing(executor->context, b); \
    }

_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_add, +);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_subtract, -);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_multiply, *);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_divide, /);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_lessThanOrEqualTo, <=);
_VOXEL_BUILTINS_CORE_NUMBER_OPERATOR(voxel_builtins_core_greaterThanOrEqualTo, >=);

void voxel_builtins_core_modulo(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int b = voxel_popNumberInt(executor);
    voxel_Int a = voxel_popNumberInt(executor);

    if (b == 0) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, a % b));
}

void voxel_builtins_core_equal(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (!a || !b) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newBoolean(executor->context, voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_notEqual(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* b = voxel_pop(executor);
    voxel_Thing* a = voxel_pop(executor);

    if (!a || !b) {
        return voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newBoolean(executor->context, !voxel_compareThings(a, b)));

    voxel_unreferenceThing(executor->context, a);
    voxel_unreferenceThing(executor->context, b);
}

void voxel_builtins_core_negate(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* value = voxel_popNumber(executor);

    if (!value) {
        voxel_pushNull(executor);
    }

    voxel_push(executor, voxel_newNumberFloat(executor->context, -voxel_getNumberFloat(value)));

    voxel_unreferenceThing(executor->context, value);
}

#endif

// src/builtins/core/objects.h

#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newObject(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);

    voxel_push(executor, voxel_newObject(executor->context));
}

void voxel_builtins_core_getObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        voxel_unreferenceThing(executor->context, key);
        voxel_unreferenceThing(executor->context, object);

        return voxel_pushNull(executor);
    }

    voxel_Thing* value = objectItem->value;

    value->referenceCount++;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    voxel_push(executor, value);
}

void voxel_builtins_core_setObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_setObjectItem(executor->context, object, key, value);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_removeObjectItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_removeObjectItem(executor->context, object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    voxel_pushNull(executor);
}

void voxel_builtins_core_getObjectItemGetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    if (!objectItem) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->getterFunction) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* getterFunction = descriptor->getterFunction;

    getterFunction->referenceCount++;

    voxel_push(executor, getterFunction);
}

void voxel_builtins_core_setObjectItemGetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        if (VOXEL_IS_ERROR(objectItemResult)) {
            return voxel_pushNull(executor);
        }

        objectItem = objectItemResult.value;
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->getterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->getterFunction);
    }

    descriptor->getterFunction = value;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_getObjectItemSetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 2) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    if (!objectItem) {
        return voxel_pushNull(executor);
    }

    voxel_ObjectItemDescriptor* descriptor = objectItem->descriptor;

    if (!descriptor || !descriptor->setterFunction) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* setterFunction = descriptor->setterFunction;

    setterFunction->referenceCount++;

    voxel_push(executor, setterFunction);
}

void voxel_builtins_core_setObjectItemSetter(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* key = voxel_pop(executor);
    voxel_Thing* object = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!object || object->type != VOXEL_TYPE_OBJECT || argCount < 3) {
        return;
    }

    voxel_ObjectItem* objectItem = voxel_getObjectItem(object, key);

    if (!objectItem) {
        VOXEL_ERRORABLE objectItemResult = voxel_setObjectItem(executor->context, object, key, voxel_newNull(executor->context));

        if (VOXEL_IS_ERROR(objectItemResult)) {
            return voxel_pushNull(executor);
        }

        objectItem = objectItemResult.value;
    }

    voxel_ObjectItemDescriptor* descriptor = voxel_ensureObjectItemDescriptor(executor->context, objectItem);

    if (descriptor->setterFunction) {
        voxel_unreferenceThing(executor->context, descriptor->setterFunction);
    }

    descriptor->setterFunction = value;

    voxel_unreferenceThing(executor->context, key);
    voxel_unreferenceThing(executor->context, object);

    value->referenceCount++;

    voxel_push(executor, value);
}

void voxel_builtins_core_getObjectLength(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT) {
        return;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getObjectLength(object)));

    voxel_unreferenceThing(executor->context, object);
}

void voxel_builtins_core_getObjectPrototypes(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* object = voxel_pop(executor);

    if (!object || object->type != VOXEL_TYPE_OBJECT) {
        return;
    }

    voxel_Thing* prototypes = voxel_getObjectPrototypes(executor->context, object);

    prototypes->referenceCount++;

    voxel_push(executor, prototypes);

    voxel_unreferenceThing(executor->context, object);
}

#endif

// src/builtins/core/lists.h

#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_newList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);

    voxel_push(executor, voxel_newList(executor->context));
}

void voxel_builtins_core_newListOf(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_newList(executor->context);

    for (voxel_Count i = 0; i < argCount; i++) {
        voxel_Thing* item = voxel_pop(executor);

        if (!item) {
            break;
        }

        voxel_insertIntoList(executor->context, list, 0, item);
        voxel_unreferenceThing(executor->context, item);
    }

    voxel_push(executor, list);
}

void voxel_builtins_core_getListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    voxel_List* listValue = list->value;

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0 || index >= listValue->length) {
        voxel_unreferenceThing(executor->context, list);

        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(executor->context, list, index);

    if (VOXEL_IS_ERROR(listItemResult)) {
        return voxel_pushNull(executor);
    }

    voxel_ListItem* listItem = listItemResult.value;

    if (!listItem) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* value = listItem->value;

    value->referenceCount++;

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, value);
}

void voxel_builtins_core_setListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 3) {
        return;
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return;
    }

    VOXEL_ERRORABLE getListItemResult = voxel_getListItem(executor->context, list, index);

    if (VOXEL_IS_ERROR(getListItemResult)) {
        return voxel_pushNull(executor);
    }

    voxel_ListItem* listItem = getListItemResult.value;

    if (!listItem) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* currentValue = listItem->value;

    voxel_unreferenceThing(executor->context, currentValue);

    listItem->value = value;
    value->referenceCount += 2;

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, value);
}

void voxel_builtins_core_removeListItem(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        voxel_unreferenceThing(executor->context, list);

        return voxel_pushNull(executor);
    }

    voxel_removeListItem(executor->context, list, index);

    voxel_unreferenceThing(executor->context, list);

    voxel_pushNull(executor);
}

void voxel_builtins_core_pushOntoList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    if (VOXEL_IS_ERROR(voxel_pushOntoList(executor->context, list, value))) {
        return voxel_pushNull(executor);
    }

    value->referenceCount--;

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));
}

void voxel_builtins_core_popFromList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return voxel_pushNull(executor);
    }

    voxel_List* listValue = list->value;
    voxel_ListItem* lastItem = listValue->lastItem;

    if (!lastItem) {
        voxel_unreferenceThing(executor->context, list);

        return voxel_pushNull(executor);
    }

    voxel_Thing* lastThing = lastItem->value;

    lastThing->referenceCount++;

    VOXEL_ERRORABLE result = voxel_popFromList(executor->context, list);

    if (VOXEL_IS_ERROR(result)) {
        return voxel_pushNull(executor);
    }

    voxel_unreferenceThing(executor->context, list);

    voxel_push(executor, lastThing);
}

void voxel_builtins_core_insertIntoList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Int index = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);
    voxel_Thing* value = voxel_peek(executor, 0); // Keep as return value

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 3) {
        return;
    }

    if (index < 0) {
        index = voxel_getListLength(list) + index;
    }

    if (index < 0) {
        return;
    }

    if (VOXEL_IS_ERROR(voxel_insertIntoList(executor->context, list, index, value))) {
        return;
    }

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_getListLength(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_joinList(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* delimeter = voxel_popString(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST || argCount < 2) {
        return voxel_pushNull(executor);
    }

    VOXEL_ERRORABLE result = voxel_joinList(executor->context, list, delimeter);

    if (VOXEL_IS_ERROR(result)) {
        return voxel_pushNull(executor);
    }

    voxel_unreferenceThing(executor->context, list);
    voxel_unreferenceThing(executor->context, delimeter);

    voxel_push(executor, result.value);
}

#endif

// src/builtins/core/core.h

#ifdef VOXEL_BUILTINS_CORE

void voxel_builtins_core_log(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (thing) {
        voxel_logThing(executor->context, thing);

        voxel_unreferenceThing(executor->context, thing);
    }

    voxel_pushNull(executor);
}

void voxel_builtins_core_params(voxel_Executor* executor) {
    voxel_Int required = voxel_popNumberInt(executor);
    voxel_Int actual = voxel_popNumberInt(executor);

    while (required < actual) {
        voxel_popVoid(executor);

        actual--;
    }

    while (required > actual) {
        voxel_pushNull(executor);

        actual++;
    }
}

void voxel_builtins_core_getType(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* thing = voxel_pop(executor);

    if (!thing) {
        return voxel_pushNull(executor);
    }

    voxel_Byte thingType[2] = {0x00, 0x00};

    switch (thing->type) {
        case VOXEL_TYPE_NULL: thingType[0] = 'n'; break;
        case VOXEL_TYPE_BOOLEAN: thingType[0] = 't'; break;
        case VOXEL_TYPE_BYTE: thingType[0] = 'b'; break;
        case VOXEL_TYPE_FUNCTION: thingType[0] = '@'; break;
        case VOXEL_TYPE_CLOSURE: thingType[0] = 'C'; break;
        case VOXEL_TYPE_NUMBER: thingType[0] = '%'; break;
        case VOXEL_TYPE_BUFFER: thingType[0] = 'B'; break;
        case VOXEL_TYPE_STRING: thingType[0] = '"'; break;
        case VOXEL_TYPE_OBJECT: thingType[0] = 'O'; break;
        case VOXEL_TYPE_LIST: thingType[0] = 'L'; break;
    }

    voxel_unreferenceThing(executor->context, thing);

    voxel_push(executor, voxel_newStringTerminated(executor->context, thingType));
}

void voxel_builtins_core_toClosure(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* environment = voxel_pop(executor);
    voxel_Thing* function = voxel_pop(executor);

    if (
        !environment || environment->type != VOXEL_TYPE_OBJECT ||
        !function || function->type != VOXEL_TYPE_FUNCTION ||
        argCount < 2
    ) {
        return voxel_pushNull(executor);
    }

    if (voxel_getFunctionType(executor->context, function) != VOXEL_FUNCTION_TYPE_POS_REF) {
        voxel_unreferenceThing(executor->context, environment);
        voxel_unreferenceThing(executor->context, function);

        return voxel_pushNull(executor);
    }

    voxel_push(
        executor,
        voxel_newClosure(executor->context, (voxel_Position)(voxel_IntPtr)function->value, environment)
    );

    voxel_unreferenceThing(executor->context, environment);
    voxel_unreferenceThing(executor->context, function);
}

void voxel_builtins_core_pushArgs(voxel_Executor* executor) {
    voxel_Int argCount = voxel_popNumberInt(executor);
    voxel_Thing* list = voxel_pop(executor);

    if (!list || list->type != VOXEL_TYPE_LIST) {
        return;
    }

    voxel_List* listValue = list->value;
    voxel_ListItem* currentItem = listValue->firstItem;

    while (currentItem) {
        voxel_Thing* currentThing = currentItem->value;

        voxel_push(executor, currentThing);

        currentThing->referenceCount++;

        currentItem = currentItem->nextItem;
    }

    voxel_push(executor, voxel_newNumberInt(executor->context, voxel_getListLength(list)));

    voxel_unreferenceThing(executor->context, list);
}

void voxel_builtins_core_getItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 2) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_getObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_getListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_setItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 3) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_setObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_setListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Value
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_removeItem(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 2) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 2);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_removeObjectItem(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_removeListItem(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing
    voxel_popVoid(executor); // Key

    voxel_pushNull(executor);
}

void voxel_builtins_core_getLength(voxel_Executor* executor) {
    voxel_Thing* argCount = voxel_peek(executor, 0);

    if (voxel_getNumberInt(argCount) < 1) {
        return voxel_pushNull(executor);
    }

    voxel_Thing* thing = voxel_peek(executor, 1);

    switch (thing->type) {
        case VOXEL_TYPE_OBJECT: return voxel_builtins_core_getObjectLength(executor);
        case VOXEL_TYPE_LIST: return voxel_builtins_core_getListLength(executor);
    }

    voxel_popVoid(executor); // Arg count
    voxel_popVoid(executor); // Thing

    voxel_pushNull(executor);
}

void voxel_builtins_core(voxel_Context* context) {
    voxel_defineBuiltin(context, ".log", &voxel_builtins_core_log);
    voxel_defineBuiltin(context, ".P", &voxel_builtins_core_params);
    voxel_defineBuiltin(context, ".T", &voxel_builtins_core_getType);
    voxel_defineBuiltin(context, ".C", &voxel_builtins_core_toClosure);
    voxel_defineBuiltin(context, ".Au", &voxel_builtins_core_pushArgs);

    voxel_defineBuiltin(context, ".+", &voxel_builtins_core_add);
    voxel_defineBuiltin(context, ".-", &voxel_builtins_core_subtract);
    voxel_defineBuiltin(context, ".*", &voxel_builtins_core_multiply);
    voxel_defineBuiltin(context, "./", &voxel_builtins_core_divide);
    voxel_defineBuiltin(context, ".%", &voxel_builtins_core_modulo);
    voxel_defineBuiltin(context, ".-x", &voxel_builtins_core_negate);
    voxel_defineBuiltin(context, ".<=", &voxel_builtins_core_lessThanOrEqualTo);
    voxel_defineBuiltin(context, ".>=", &voxel_builtins_core_greaterThanOrEqualTo);

    voxel_defineBuiltin(context, ".Tg", &voxel_builtins_core_getItem);
    voxel_defineBuiltin(context, ".Ts", &voxel_builtins_core_setItem);
    voxel_defineBuiltin(context, ".Tr", &voxel_builtins_core_removeItem);
    voxel_defineBuiltin(context, ".Tl", &voxel_builtins_core_getLength);

    voxel_defineBuiltin(context, ".O", &voxel_builtins_core_newObject);
    voxel_defineBuiltin(context, ".Og", &voxel_builtins_core_getObjectItem);
    voxel_defineBuiltin(context, ".Os", &voxel_builtins_core_setObjectItem);
    voxel_defineBuiltin(context, ".Or", &voxel_builtins_core_removeObjectItem);
    voxel_defineBuiltin(context, ".Ogg", &voxel_builtins_core_getObjectItemGetter);
    voxel_defineBuiltin(context, ".Osg", &voxel_builtins_core_setObjectItemGetter);
    voxel_defineBuiltin(context, ".Ogs", &voxel_builtins_core_getObjectItemSetter);
    voxel_defineBuiltin(context, ".Oss", &voxel_builtins_core_setObjectItemSetter);
    voxel_defineBuiltin(context, ".Ol", &voxel_builtins_core_getObjectLength);
    voxel_defineBuiltin(context, ".Op", &voxel_builtins_core_getObjectPrototypes);

    voxel_defineBuiltin(context, ".L", &voxel_builtins_core_newList);
    voxel_defineBuiltin(context, ".Lo", &voxel_builtins_core_newListOf);
    voxel_defineBuiltin(context, ".Lg", &voxel_builtins_core_getListItem);
    voxel_defineBuiltin(context, ".Ls", &voxel_builtins_core_setListItem);
    voxel_defineBuiltin(context, ".Lr", &voxel_builtins_core_removeListItem);
    voxel_defineBuiltin(context, ".Lu", &voxel_builtins_core_pushOntoList);
    voxel_defineBuiltin(context, ".Lp", &voxel_builtins_core_popFromList);
    voxel_defineBuiltin(context, ".Li", &voxel_builtins_core_insertIntoList);
    voxel_defineBuiltin(context, ".Ll", &voxel_builtins_core_getListLength);
    voxel_defineBuiltin(context, ".Lj", &voxel_builtins_core_joinList);
}

#else

void voxel_builtins_core(voxel_Context* context) {}

#endif

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

// src/contexts.h

voxel_Context* voxel_newContext() {
    voxel_Context* context = VOXEL_MALLOC(sizeof(voxel_Context)); VOXEL_TAG_MALLOC(voxel_Context);

    context->isInitialised = VOXEL_FALSE;
    context->code = VOXEL_NULL;
    context->codeLength = 0;
    context->builtins = VOXEL_MALLOC(0); VOXEL_TAG_MALLOC_SIZE("voxel_Context->builtins", 0);
    context->builtinCount = 0;
    context->tokenisationState = VOXEL_STATE_NONE;
    context->firstTrackedThing = VOXEL_NULL;
    context->lastTrackedThing = VOXEL_NULL;
    context->firstExecutor = VOXEL_NULL;
    context->lastExecutor = VOXEL_NULL;
    context->globalScope = voxel_newScope(context, VOXEL_NULL);

    voxel_newExecutor(context);

    voxel_builtins_core(context);

    return context;
}

VOXEL_ERRORABLE voxel_initContext(voxel_Context* context) {
    if (context->isInitialised) {
        return VOXEL_OK;
    }

    #ifdef VOXEL_MAGIC
        if (context->codeLength < VOXEL_MAGIC_SIZE) {
            VOXEL_THROW(VOXEL_ERROR_INVALID_MAGIC);
        }

        voxel_Byte* magic = (voxel_Byte[VOXEL_MAGIC_SIZE]) {VOXEL_MAGIC};

        for (voxel_Count i = 0; i < VOXEL_MAGIC_SIZE; i++) {
            if (context->code[i] != magic[i]) {
                VOXEL_THROW(VOXEL_ERROR_INVALID_MAGIC);
            }
        }
    #endif

    context->isInitialised = VOXEL_TRUE;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_stepContext(voxel_Context* context) {
    VOXEL_ASSERT(context->isInitialised, VOXEL_ERROR_NOT_INITIALISED);

    voxel_Executor* currentExecutor = context->firstExecutor;

    while (currentExecutor) {
        VOXEL_MUST(voxel_stepExecutor(currentExecutor));

        currentExecutor = currentExecutor->nextExecutor;
    }

    voxel_removeUnusedThings(context);

    return VOXEL_OK;
}

voxel_Bool voxel_anyContextsRunning(voxel_Context* context) {
    voxel_Executor* currentExecutor = context->firstExecutor;

    while (currentExecutor) {
        if (!currentExecutor->isRunning) {
            return VOXEL_FALSE;
        }

        currentExecutor = currentExecutor->nextExecutor;
    }

    return VOXEL_TRUE;
}

VOXEL_ERRORABLE voxel_defineBuiltin(voxel_Context* context, voxel_Byte* name, voxel_Builtin builtin) {
    voxel_Thing* key = voxel_newStringTerminated(context, name);

    context->builtinCount++;

    voxel_Count newSize = context->builtinCount * sizeof(voxel_Builtin);

    context->builtins = VOXEL_REALLOC(context->builtins, newSize); VOXEL_TAG_REALLOC("voxel_Context->builtins", newSize - sizeof(voxel_Builtin), newSize);
    context->builtins[context->builtinCount - 1] = builtin;

    voxel_Thing* function = voxel_newFunctionBuiltin(context, context->builtinCount - 1);

    VOXEL_MUST(voxel_setScopeItem(context->globalScope, key, function));

    return VOXEL_OK;
}

// src/things.h

voxel_Thing* voxel_newThing(voxel_Context* context) {
    voxel_Thing* thing = VOXEL_MALLOC(sizeof(voxel_Thing)); VOXEL_TAG_MALLOC(voxel_Thing);

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

VOXEL_ERRORABLE voxel_destroyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_destroyNull(thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_destroyBoolean(thing);
        case VOXEL_TYPE_BYTE: return voxel_destroyByte(thing);
        case VOXEL_TYPE_FUNCTION: return voxel_destroyFunction(thing);
        case VOXEL_TYPE_CLOSURE: return voxel_destroyClosure(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_destroyNumber(thing);
        case VOXEL_TYPE_BUFFER: return voxel_destroyBuffer(thing);
        case VOXEL_TYPE_STRING: return voxel_destroyString(thing);
        case VOXEL_TYPE_OBJECT: return voxel_destroyObject(context, thing);
        case VOXEL_TYPE_LIST: return voxel_destroyList(context, thing);
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing) {
    if (!thing) {
        return VOXEL_OK;
    }

    if (thing->referenceCount > 0) {
        thing->referenceCount--;
    }

    if (thing->referenceCount > 0) {
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

    if (thing->nextTrackedThing) {
        thing->nextTrackedThing->previousTrackedThing = thing->previousTrackedThing;
    }

    VOXEL_MUST(voxel_destroyThing(context, thing));

    return VOXEL_OK;
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
    if (a == b) {
        return VOXEL_TRUE;
    }

    if (!voxel_compareThingTypes(a, b)) {
        return VOXEL_FALSE;
    }

    switch (a->type) {
        case VOXEL_TYPE_NULL: return voxel_compareNulls(a, b);
        case VOXEL_TYPE_BOOLEAN: return voxel_compareBooleans(a, b);
        case VOXEL_TYPE_BYTE: return voxel_compareBytes(a, b);
        case VOXEL_TYPE_FUNCTION: return voxel_compareFunctions(a, b);
        case VOXEL_TYPE_CLOSURE: return voxel_compareClosures(a, b);
        case VOXEL_TYPE_NUMBER: return voxel_compareNumbers(a, b);
        case VOXEL_TYPE_BUFFER: return voxel_compareBuffers(a, b);
        case VOXEL_TYPE_STRING: return voxel_compareStrings(a, b);
        case VOXEL_TYPE_OBJECT: return voxel_compareObjects(a, b);
        case VOXEL_TYPE_LIST: return voxel_compareLists(a, b);
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning `VOXEL_FALSE` for now\n");

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

        case VOXEL_TYPE_LIST:
            voxel_lockList(thing);
            break;
    }
}

voxel_Thing* voxel_copyThing(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_copyNull(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_copyBoolean(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_copyByte(context, thing);
        case VOXEL_TYPE_FUNCTION: return voxel_copyFunction(context, thing);
        case VOXEL_TYPE_CLOSURE: return voxel_copyClosure(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_copyNumber(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_copyBuffer(context, thing);
        case VOXEL_TYPE_STRING: return voxel_copyString(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_copyObject(context, thing);
        case VOXEL_TYPE_LIST: return voxel_copyList(context, thing);
    }

    VOXEL_DEBUG_LOG("Thing comparison not implemented; returning null thing for now\n");

    return voxel_newNull(context);
}

VOXEL_ERRORABLE voxel_thingToString(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullToString(context, thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToString(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToString(context, thing);
        case VOXEL_TYPE_FUNCTION: return voxel_functionToString(context, thing);
        case VOXEL_TYPE_CLOSURE: return voxel_closureToString(context, thing);
        case VOXEL_TYPE_NUMBER: return voxel_numberToString(context, thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferToString(context, thing);
        case VOXEL_TYPE_STRING: return VOXEL_OK_RET(voxel_copyString(context, thing));
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
        case VOXEL_TYPE_LIST: return voxel_listToString(context, thing);
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_thingToVxon(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_BYTE: return voxel_byteToVxon(context, thing);

        case VOXEL_TYPE_FUNCTION:
            #ifdef VOXEL_DEBUG_EXECUTORS
                break;
            #else
                VOXEL_THROW(VOXEL_ERROR_CANNOT_CONVERT_THING);
            #endif

        case VOXEL_TYPE_BUFFER: return voxel_bufferToVxon(context, thing);
        case VOXEL_TYPE_STRING: return voxel_stringToVxon(context, thing);
        case VOXEL_TYPE_OBJECT: return voxel_objectToVxon(context, thing);
        case VOXEL_TYPE_LIST: return voxel_listToVxon(context, thing);
    }

    return voxel_thingToString(context, thing);
}

VOXEL_ERRORABLE voxel_thingToNumber(voxel_Context* context, voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return VOXEL_OK_RET(voxel_newNumberInt(context, 0));
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanToNumber(context, thing);
        case VOXEL_TYPE_BYTE: return voxel_byteToNumber(context, thing);
        case VOXEL_TYPE_FUNCTION: case VOXEL_TYPE_CLOSURE: return VOXEL_OK_RET(voxel_newNumberInt(context, 1));
        case VOXEL_TYPE_NUMBER: return VOXEL_OK_RET(voxel_copyNumber(context, thing));
        case VOXEL_TYPE_BUFFER: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getBufferSize(thing)));
        case VOXEL_TYPE_STRING: return voxel_stringToNumber(context, thing);
        case VOXEL_TYPE_OBJECT: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getObjectLength(thing)));
        case VOXEL_TYPE_LIST: return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getListLength(thing)));
    }

    VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
}

VOXEL_ERRORABLE voxel_thingToByte(voxel_Context* context, voxel_Thing* thing) {
    if (thing->type == VOXEL_TYPE_BYTE) {
        return VOXEL_OK_RET(voxel_copyByte(context, thing));
    }

    VOXEL_ERRORABLE number = voxel_thingToNumber(context, thing); VOXEL_MUST(number);

    return voxel_numberToByte(context, number.value);
}

voxel_Bool voxel_thingIsTruthy(voxel_Thing* thing) {
    switch (thing->type) {
        case VOXEL_TYPE_NULL: return voxel_nullIsTruthy(thing);
        case VOXEL_TYPE_BOOLEAN: return voxel_booleanIsTruthy(thing);
        case VOXEL_TYPE_BYTE: return voxel_byteIsTruthy(thing);
        case VOXEL_TYPE_FUNCTION: return voxel_functionIsTruthy(thing);
        case VOXEL_TYPE_CLOSURE: return voxel_closureIsTruthy(thing);
        case VOXEL_TYPE_NUMBER: return voxel_numberIsTruthy(thing);
        case VOXEL_TYPE_BUFFER: return voxel_bufferIsTruthy(thing);
        case VOXEL_TYPE_STRING: return voxel_stringIsTruthy(thing);
        case VOXEL_TYPE_OBJECT: return voxel_objectIsTruthy(thing);
        case VOXEL_TYPE_LIST: return voxel_listIsTruthy(thing);
    }

    VOXEL_DEBUG_LOG("Thing truthiness not implemented; returning null thing for now\n");

    return VOXEL_FALSE;
}

VOXEL_ERRORABLE voxel_logThing(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ERRORABLE string = voxel_thingToString(context, thing); VOXEL_MUST(string);

    voxel_logString(string.value);

    VOXEL_MUST(voxel_unreferenceThing(context, string.value));
}

// src/null.h

voxel_Thing* voxel_newNull(voxel_Context* context) {
    return voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NULL);
}

VOXEL_ERRORABLE voxel_destroyNull(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_NULL);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareNulls(voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_TRUE;
}

voxel_Thing* voxel_copyNull(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NULL);
}

VOXEL_ERRORABLE voxel_nullToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "null"));
}

voxel_Bool voxel_nullIsTruthy(voxel_Thing* thing) {
    return VOXEL_FALSE;
}

// src/booleans.h

voxel_Thing* voxel_newBoolean(voxel_Context* context, voxel_Bool value) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_BOOLEAN);

    thing->type = VOXEL_TYPE_BOOLEAN;
    thing->value = value ? (void*)0x01 : (void*)0x00;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyBoolean(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_BOOLEAN);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBooleans(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyBoolean(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newBoolean(context, (voxel_IntPtr)thing->value);
}

VOXEL_ERRORABLE voxel_booleanToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, thing->value ? "true" : "false"));
}

VOXEL_ERRORABLE voxel_booleanToNumber(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newNumberInt(context, thing->value ? 1 : 0));
}

voxel_Bool voxel_booleanIsTruthy(voxel_Thing* thing) {
    return !!thing->value;
}

// src/bytes.h

voxel_Thing* voxel_newByte(voxel_Context* context, voxel_Byte value) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_BYTE);

    thing->type = VOXEL_TYPE_BYTE;
    thing->value = (void*)(voxel_IntPtr)value;
}

VOXEL_ERRORABLE voxel_destroyByte(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_BYTE);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBytes(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyByte(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newByte(context, (voxel_IntPtr)thing->value);
}

VOXEL_ERRORABLE voxel_byteToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Byte bytes[1] = {(voxel_IntPtr)thing->value};

    return VOXEL_OK_RET(voxel_newString(context, 1, bytes));
}

VOXEL_ERRORABLE voxel_byteToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* string = voxel_newStringTerminated(context, "byte(0x");
    voxel_Thing* number = voxel_newNumberInt(context, (voxel_IntPtr)thing->value);
    VOXEL_ERRORABLE hexString = voxel_numberToBaseString(context, number, 16, 2); VOXEL_MUST(hexString);

    VOXEL_MUST(voxel_appendToString(context, string, hexString.value));
    VOXEL_MUST(voxel_appendByteToString(context, string, ')'));

    VOXEL_MUST(voxel_unreferenceThing(context, number));
    VOXEL_MUST(voxel_unreferenceThing(context, hexString.value));

    return VOXEL_OK_RET(string);
}

VOXEL_ERRORABLE voxel_byteToNumber(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newNumberInt(context, (voxel_IntPtr)thing->value));
}

voxel_Bool voxel_byteIsTruthy(voxel_Thing* thing) {
    return thing->value != 0x00;
}

// src/functions.h

voxel_Thing* voxel_newFunctionBuiltin(voxel_Context* context, voxel_Count builtinFunctionIndex) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

    builtinFunctionIndex++;
    builtinFunctionIndex *= -1;

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)builtinFunctionIndex;

    return thing;
}

voxel_Thing* voxel_newFunctionPosRef(voxel_Context* context, voxel_Position positionReference) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

    thing->type = VOXEL_TYPE_FUNCTION;
    thing->value = (void*)(voxel_IntPtr)positionReference;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyFunction(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_FUNCTION);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareFunctions(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyFunction(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* newThing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_FUNCTION);

    newThing->type = VOXEL_TYPE_FUNCTION;
    newThing->value = thing->value;

    return newThing;
}

VOXEL_ERRORABLE voxel_functionToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(
        context,
        voxel_getFunctionType(context, thing) == VOXEL_FUNCTION_TYPE_BUILTIN ?
        "(builtin function)" :
        "(function)"
    ));
}

voxel_FunctionType voxel_getFunctionType(voxel_Context* context, voxel_Thing* thing) {    
    if ((voxel_Int)(voxel_IntPtr)thing->value < 0) {
        return VOXEL_FUNCTION_TYPE_BUILTIN;
    }

    return VOXEL_FUNCTION_TYPE_POS_REF;
}

voxel_Bool voxel_functionIsTruthy(voxel_Thing* thing) {
    return VOXEL_TRUE;
}

// src/closures.h

voxel_Thing* voxel_newClosure(voxel_Context* context, voxel_Position positionReference, voxel_Thing* environment) {
    voxel_Closure* closure = VOXEL_MALLOC(sizeof(voxel_Closure)); VOXEL_TAG_MALLOC(voxel_Closure);

    closure->position = positionReference;
    closure->environment = environment;

    environment->referenceCount++;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_CLOSURE);

    thing->type = VOXEL_TYPE_CLOSURE;
    thing->value = closure;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyClosure(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_CLOSURE);

    voxel_Closure* closure = thing->value;

    VOXEL_MUST(voxel_unreferenceThing(context, closure->environment));

    VOXEL_FREE(closure); VOXEL_TAG_FREE(voxel_Closure);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareClosures(voxel_Thing* a, voxel_Thing* b) {
    voxel_Closure* aClosure = a->value;
    voxel_Closure* bClosure = b->value;

    if (aClosure->position != bClosure->position) {
        return VOXEL_FALSE;
    }

    return voxel_compareObjects(aClosure->environment, bClosure->environment);
}

voxel_Thing* voxel_copyClosure(voxel_Context* context, voxel_Thing* thing) {
    voxel_Closure* closure = thing->value;

    return voxel_newClosure(context, closure->position, closure->environment);
}

VOXEL_ERRORABLE voxel_closureToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, "(closure function)"));
}

voxel_Bool voxel_closureIsTruthy(voxel_Thing* thing) {
    return VOXEL_TRUE;
}

// src/numbers.h

voxel_Thing* voxel_newNumberInt(voxel_Context* context, voxel_Int value) {
    voxel_Number* number = VOXEL_MALLOC(sizeof(voxel_Number)); VOXEL_TAG_MALLOC(voxel_Number);

    number->type = VOXEL_NUMBER_TYPE_INT;
    number->value.asInt = value;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_NUMBER);

    thing->type = VOXEL_TYPE_NUMBER;
    thing->value = number;

    return thing;
}

voxel_Thing* voxel_newNumberFloat(voxel_Context* context, voxel_Float value) {
    voxel_Number* number = VOXEL_MALLOC(sizeof(voxel_Number)); VOXEL_TAG_MALLOC(voxel_Number);

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

    voxel_Number* number = thing->value;

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

    voxel_Number* number = thing->value;

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
    voxel_Number* aNumber = a->value;
    voxel_Number* bNumber = b->value;

    if (aNumber->type == VOXEL_NUMBER_TYPE_INT && bNumber->type == VOXEL_NUMBER_TYPE_INT) {
        return aNumber->value.asInt == bNumber->value.asInt;
    }

    return voxel_getNumberFloat(a) == voxel_getNumberFloat(b);
}

voxel_Thing* voxel_copyNumber(voxel_Context* context, voxel_Thing* thing) {
    voxel_Number* number = thing->value;

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
            return VOXEL_OK_RET(voxel_newStringTerminated(context, isNegative ? "-infinity" : "infinity"));
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

        voxel_appendToString(context, string, exponentString.value);

        voxel_unreferenceThing(context, exponentNumber);
        voxel_unreferenceThing(context, exponentString.value);
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

    VOXEL_MUST(voxel_padStringEnd(context, string, minSize, '0'));

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
    voxel_Number* number = thing->value;

    switch (number->type) {
        case VOXEL_NUMBER_TYPE_INT:
            return number->value.asInt != 0;

        case VOXEL_NUMBER_TYPE_FLOAT:
            return number->value.asFloat != 0.0;
    }
}

// src/buffers.h

voxel_Thing* voxel_newBuffer(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_Buffer* buffer = VOXEL_MALLOC(sizeof(voxel_Buffer)); VOXEL_TAG_MALLOC(voxel_Buffer);

    buffer->size = size;
    buffer->value = VOXEL_MALLOC(size); VOXEL_TAG_MALLOC_SIZE("voxel_Buffer->value", size);

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_BUFFER);

    thing->type = VOXEL_TYPE_BUFFER;
    thing->value = buffer;

    if (data != VOXEL_NULL) {
        voxel_copy(data, buffer->value, size);
    } else {
        for (voxel_Count i = 0; i < size; i++) {
            buffer->value[i] = 0;
        }
    }

    return thing;
}

VOXEL_ERRORABLE voxel_destroyBuffer(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_BUFFER);

    voxel_Buffer* buffer = thing->value;

    VOXEL_FREE(buffer->value); VOXEL_TAG_FREE_SIZE("voxel_Buffer->value", buffer->size);
    VOXEL_FREE(buffer); VOXEL_TAG_FREE(voxel_Buffer);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareBuffers(voxel_Thing* a, voxel_Thing* b) {
    voxel_Buffer* aBuffer = a->value;
    voxel_Buffer* bBuffer = b->value;

    return voxel_compare(aBuffer->value, bBuffer->value, aBuffer->size, bBuffer->size);
}

voxel_Thing* voxel_copyBuffer(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;
    
    return voxel_newBuffer(context, buffer->size, buffer->value);
}

VOXEL_ERRORABLE voxel_bufferToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    return VOXEL_OK_RET(voxel_newString(context, buffer->size, buffer->value));
}

VOXEL_ERRORABLE voxel_bufferToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "buffer([");
    voxel_Thing* hexPrefix = voxel_newStringTerminated(context, "0x");
    voxel_Thing* suffix = voxel_newStringTerminated(context, "])");

    for (voxel_Count i = 0; i < buffer->size; i++) {
        voxel_Thing* number = voxel_newNumberInt(context, buffer->value[i]);
        VOXEL_ERRORABLE hexString = voxel_numberToBaseString(context, number, 16, 2); VOXEL_MUST(hexString);

        VOXEL_MUST(voxel_appendToString(context, string, hexPrefix));
        VOXEL_MUST(voxel_appendToString(context, string, hexString.value));

        if (i < buffer->size - 1) {
            VOXEL_MUST(voxel_appendByteToString(context, string, ','));
        }

        VOXEL_MUST(voxel_unreferenceThing(context, number));
        VOXEL_MUST(voxel_unreferenceThing(context, hexString.value));
    }

    VOXEL_MUST(voxel_appendToString(context, string, suffix));

    VOXEL_MUST(voxel_unreferenceThing(context, hexPrefix));
    VOXEL_MUST(voxel_unreferenceThing(context, suffix));

    return VOXEL_OK_RET(string);
}

voxel_Bool voxel_bufferIsTruthy(voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    return buffer->size != 0;
}

voxel_Count voxel_getBufferSize(voxel_Thing* thing) {
    voxel_Buffer* buffer = thing->value;

    return buffer->size;
}

// src/strings.h

voxel_Thing* voxel_newString(voxel_Context* context, voxel_Count size, voxel_Byte* data) {
    voxel_String* string = VOXEL_MALLOC(sizeof(voxel_String)); VOXEL_TAG_MALLOC(voxel_String);

    string->size = size;
    string->value = VOXEL_MALLOC(size); VOXEL_TAG_MALLOC_SIZE("voxel_String->value", size);

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_STRING);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = string;

    if (size > 0) {
        voxel_copy(data, string->value, size);
    }

    return thing;
}

voxel_Thing* voxel_newStringTerminated(voxel_Context* context, voxel_Byte* data) {
    voxel_Count size = 0;

    while (data[size] != '\0') {
        size++;
    }

    return voxel_newString(context, size, data);
}

VOXEL_ERRORABLE voxel_destroyString(voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_STRING);

    voxel_String* string = thing->value;

    VOXEL_FREE(string->value); VOXEL_TAG_FREE_SIZE("voxel_String->value", string->size);
    VOXEL_FREE(string); VOXEL_TAG_FREE(voxel_String);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareStrings(voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    return voxel_compare(aString->value, bString->value, aString->size, bString->size);
}

voxel_Thing* voxel_copyString(voxel_Context* context, voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return voxel_newString(context, string->size, string->value);
}

// @source https://stackoverflow.com/a/4392789
VOXEL_ERRORABLE voxel_stringToNumber(voxel_Context* context, voxel_Thing* thing) {
    voxel_String* string = thing->value;

    voxel_Count i = 0;
    voxel_Float result = 0;
    voxel_Float factor = 1;
    voxel_Float exponent = 0;
    voxel_Float exponentIsNegative = VOXEL_FALSE;

    if (string->value[0] == '-') {
        factor = -1;
        i++;
    }

    if (i >= string->size) {
        VOXEL_THROW(VOXEL_ERROR_CANNOT_CONVERT_THING);
    }

    voxel_Bool afterPoint = VOXEL_FALSE;
    voxel_Bool hadDigit = VOXEL_FALSE;
    voxel_Bool afterExponentMark = VOXEL_FALSE;
    voxel_Bool afterExponentSign = VOXEL_FALSE;

    while (i < string->size) {
        voxel_Byte character = string->value[i];

        if (character == '.' && !afterPoint) {
            afterPoint = VOXEL_TRUE;
        } else if (
            (character == 'e' || character == 'E') &&
            !afterExponentMark && hadDigit
        ) {
            afterExponentMark = VOXEL_TRUE;
            hadDigit = VOXEL_FALSE;
        } else if (
            (character == '+' || character == '-') &&
            afterExponentMark && !afterExponentSign && !hadDigit
        ) {
            exponentIsNegative = character == '-';
            afterExponentSign = VOXEL_TRUE;
        } else if (character >= '0' && character <= '9') {
            voxel_Int digit = character - '0';

            if (afterExponentMark) {
                exponent = (exponent * 10.0) + digit;
            } else {
                if (afterPoint) {
                    factor /= 10.0;
                }

                result = (result * 10.0) + digit;
            }

            hadDigit = VOXEL_TRUE;
        } else {
            VOXEL_THROW(VOXEL_ERROR_CANNOT_CONVERT_THING);
        }

        i++;
    }

    if (!afterExponentMark) {
        exponent = 1;
    }

    if (!exponentIsNegative) {
        if (exponent == 1) {
            // Do nothing
        } else if (exponent == 0) {
            result = 1;
        } else {
            for (voxel_Count i = 0; i < exponent; i++) {
                result *= 10.0;
            }
        }
    } else {
        for (voxel_Count i = 0; i < exponent; i++) {
            result /= 10.0;
        }
    }

    return VOXEL_OK_RET(voxel_newNumberFloat(context, result * factor));
}

VOXEL_ERRORABLE voxel_stringToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_String* string = thing->value;
    voxel_Thing* vxonString = voxel_newStringTerminated(context, "\"");

    for (voxel_Count i = 0; i < string->size; i++) {
        switch (string->value[i]) {
            case '"':
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, '"'));
                break;

            case '\0':
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, '0'));
                break;

            case '\n':
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, '\\'));
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, 'n'));
                break;

            default:
                VOXEL_MUST(voxel_appendByteToString(context, vxonString, string->value[i]));
                break;
        }
    }

    VOXEL_MUST(voxel_appendByteToString(context, vxonString, '"'));

    return VOXEL_OK_RET(vxonString);
}

voxel_Bool voxel_stringIsTruthy(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return string->size != 0;
}

voxel_Count voxel_getStringSize(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    return string->size;
}

void voxel_logString(voxel_Thing* thing) {
    voxel_String* string = thing->value;

    for (voxel_Count i = 0; i < string->size; i++) {
        VOXEL_LOG_BYTE(string->value[i]);
    }
}

voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_String* resultString = VOXEL_MALLOC(sizeof(voxel_String)); VOXEL_TAG_MALLOC(voxel_String);

    resultString->size = aString->size + bString->size;
    resultString->value = VOXEL_MALLOC(resultString->size); VOXEL_TAG_MALLOC_SIZE("voxel_String->value", resultString->size);

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_STRING);

    thing->type = VOXEL_TYPE_STRING;
    thing->value = resultString;

    voxel_Count position = 0;

    for (voxel_Count i = 0; i < aString->size; i++) {
        resultString->value[position++] = aString->value[i];
    }

    for (voxel_Count i = 0; i < bString->size; i++) {
        resultString->value[position++] = bString->value[i];
    }

    return thing;
}

VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ASSERT(!a->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* aString = a->value;
    voxel_String* bString = b->value;

    voxel_Count newSize = aString->size + bString->size;

    aString->value = VOXEL_REALLOC(aString->value, newSize); VOXEL_TAG_REALLOC("voxel_String->value", aString->size, newSize);

    for (voxel_Count i = 0; i < bString->size; i++) {
        aString->value[aString->size + i] = bString->value[i];
    }

    aString->size = newSize;

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

    string->size++;
    string->value = VOXEL_REALLOC(string->value, string->size); VOXEL_TAG_REALLOC("voxel_String->value", string->size - 1, string->size);

    string->value[string->size - 1] = byte;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;
    voxel_Byte sourceString[string->size];

    voxel_copy(string->value, sourceString, string->size);

    for (voxel_Count i = 0; i < string->size; i++) {
        string->value[string->size - 1 - i] = sourceString[i];
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_cutStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count size) {
    VOXEL_MUST(voxel_reverseString(context, thing));
    VOXEL_MUST(voxel_cutStringEnd(context, thing, size));
    VOXEL_MUST(voxel_reverseString(context, thing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_cutStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count size) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    VOXEL_ASSERT(size > 0, VOXEL_ERROR_INVALID_ARG);

    voxel_String* string = thing->value;

    if (string->size < size) {
        return VOXEL_OK;
    }

    string->value = VOXEL_REALLOC(string->value, size); VOXEL_TAG_REALLOC("voxel_String->value", string->size, size);
    string->size = size;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_padStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte) {
    VOXEL_MUST(voxel_reverseString(context, thing));
    VOXEL_MUST(voxel_padStringEnd(context, thing, minSize, byte));
    VOXEL_MUST(voxel_reverseString(context, thing));

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_padStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Byte byte) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_String* string = thing->value;
    voxel_Count padding = minSize - string->size;
    voxel_Count newSize = string->size + padding;

    if (minSize <= 0) {
        return VOXEL_OK;
    }

    string->value = VOXEL_REALLOC(string->value, newSize); VOXEL_TAG_REALLOC("voxel_String->value", string->size, newSize);

    for (voxel_Count i = 0; i < padding; i++) {
        string->value[string->size + i] = byte;
    }

    string->size = newSize;

    return VOXEL_OK;
}

// src/objects.h

voxel_Thing* voxel_newObject(voxel_Context* context) {
    voxel_Object* object = VOXEL_MALLOC(sizeof(voxel_Object)); VOXEL_TAG_MALLOC(voxel_Object);

    object->length = 0;
    object->firstItem = VOXEL_NULL;
    object->lastItem = VOXEL_NULL;
    object->prototypes = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_OBJECT);

    thing->type = VOXEL_TYPE_OBJECT;
    thing->value = object;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyObject(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_OBJECT);

    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* nextItem;

    while (currentItem) {
        voxel_ObjectItemDescriptor* descriptor = currentItem->descriptor;

        VOXEL_MUST(voxel_unreferenceThing(context, currentItem->key));
        VOXEL_MUST(voxel_unreferenceThing(context, currentItem->value));

        if (descriptor) {
            VOXEL_MUST(voxel_unreferenceThing(context, descriptor->getterFunction));
            VOXEL_MUST(voxel_unreferenceThing(context, descriptor->setterFunction));

            VOXEL_FREE(descriptor); VOXEL_TAG_FREE(voxel_ObjectItemDescriptor);
        }

        nextItem = currentItem->nextItem;

        VOXEL_FREE(currentItem); VOXEL_TAG_FREE(voxel_ObjectItem);

        currentItem = nextItem;
    }

    if (object->prototypes) {
        VOXEL_MUST(voxel_unreferenceThing(context, object->prototypes));
    }

    VOXEL_FREE(object); VOXEL_TAG_FREE(voxel_Object);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareObjects(voxel_Thing* a, voxel_Thing* b) {
    voxel_Object* aObject = a->value;
    voxel_Object* bObject = b->value;

    // First check that `b` contains all values that `a` has

    voxel_ObjectItem* currentItem = aObject->firstItem;

    while (currentItem) {
        voxel_ObjectItem* objectItem = voxel_getObjectItem(b, currentItem->key);
        voxel_Bool bothAreImplicitlyNull = currentItem->value->type == VOXEL_TYPE_NULL && !objectItem;

        if (!bothAreImplicitlyNull) {
            if (!voxel_compareThings(currentItem->value, objectItem->value)) {
                return VOXEL_FALSE;
            }
        }

        currentItem = currentItem->nextItem;
    }

    // Now check in opposite direction to ensure that `b` doesn't have extra items that `a` doesn't have

    currentItem = bObject->firstItem;

    while (currentItem) {
        voxel_ObjectItem* objectItem = voxel_getObjectItem(a, currentItem->key);
        voxel_Bool bothAreImplicitlyNull = currentItem->value->type == VOXEL_TYPE_NULL && !objectItem;

        if (!bothAreImplicitlyNull) {
            if (!voxel_compareThings(currentItem->value, objectItem->value)) {
                return VOXEL_FALSE;
            }
        }

        currentItem = currentItem->nextItem;
    }

    return VOXEL_TRUE;
}

void voxel_lockObject(voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (currentItem) {
        voxel_lockThing(currentItem->value);

        currentItem = currentItem->nextItem;
    }
}

voxel_Thing* voxel_copyObject(voxel_Context* context, voxel_Thing* thing) {
    // Shallow copy only; deep copying will be handled in the Voxel standard library

    voxel_Object* object = thing->value;
    voxel_Thing* newObject = voxel_newObject(context);
    voxel_ObjectItem* currentItem = object->firstItem;

    while (currentItem) {
        voxel_setObjectItem(context, newObject, currentItem->key, currentItem->value);

        currentItem = currentItem->nextItem;
    }

    return newObject;
}

VOXEL_ERRORABLE voxel_objectToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_Thing* string = voxel_newStringTerminated(context, "{");

    while (currentItem) {
        // TODO: Prevent circular references from hanging
        VOXEL_ERRORABLE keyString = voxel_thingToVxon(context, currentItem->key); VOXEL_MUST(keyString);
        VOXEL_ERRORABLE valueString = voxel_thingToVxon(context, currentItem->value); VOXEL_MUST(valueString);

        VOXEL_MUST(voxel_appendToString(context, string, keyString.value));
        VOXEL_MUST(voxel_appendByteToString(context, string, ':'));
        VOXEL_MUST(voxel_appendToString(context, string, valueString.value));

        currentItem = currentItem->nextItem;

        if (currentItem) {
            VOXEL_MUST(voxel_appendByteToString(context, string, ','));
        }

        VOXEL_MUST(voxel_unreferenceThing(context, keyString.value));
        VOXEL_MUST(voxel_unreferenceThing(context, valueString.value));
    }

    VOXEL_MUST(voxel_appendByteToString(context, string, '}'));

    return VOXEL_OK_RET(string);
}

voxel_Bool voxel_objectIsTruthy(voxel_Thing* thing) {
    voxel_Object* object = thing->value;

    return voxel_getObjectLength(thing) != 0;
}

voxel_ObjectItem* voxel_getPrototypedObjectItem(voxel_Thing* thing, voxel_Thing* key, voxel_Count traverseDepth) {
    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            if (traverseDepth == 0) {
                return VOXEL_NULL;
            }

            voxel_Thing* prototypesThing = object->prototypes;

            if (prototypesThing == VOXEL_NULL) {
                return VOXEL_NULL;
            }

            voxel_List* prototypesList = prototypesThing->value;
            voxel_ListItem* currentPrototypeListItem = prototypesList->firstItem;

            while (currentPrototypeListItem) {
                voxel_Thing* currentPrototype = currentPrototypeListItem->value;
                voxel_ObjectItem* prototypeObjectItem = voxel_getPrototypedObjectItem(currentPrototype, key, traverseDepth - 1);

                if (prototypeObjectItem) {
                    return prototypeObjectItem;
                }

                currentPrototypeListItem = currentPrototypeListItem->nextItem;
            }

            return VOXEL_NULL;
        }

        if (voxel_compareThings(currentItem->key, key)) {
            return currentItem;
        }

        currentItem = currentItem->nextItem;
    }
}

voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* thing, voxel_Thing* key) {
    return voxel_getPrototypedObjectItem(thing, key, VOXEL_MAX_PROTOTYPE_TRAVERSE_DEPTH);
}

VOXEL_ERRORABLE voxel_setObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_Object* object = thing->value;
    voxel_ObjectItem* objectItem = voxel_getPrototypedObjectItem(thing, key, 0);

    if (objectItem) {
        VOXEL_MUST(voxel_unreferenceThing(context, objectItem->value));

        objectItem->value = value;
        value->referenceCount++;

        return VOXEL_OK_RET(objectItem);
    }

    voxel_lockThing(key);

    objectItem = VOXEL_MALLOC(sizeof(voxel_ObjectItem)); VOXEL_TAG_MALLOC(voxel_ObjectItem);

    objectItem->key = key;
    key->referenceCount++;

    objectItem->value = value;
    value->referenceCount++;

    objectItem->descriptor = VOXEL_NULL;
    objectItem->nextItem = VOXEL_NULL;

    if (!object->firstItem) {
        object->firstItem = objectItem;
    }

    if (object->lastItem) {
        object->lastItem->nextItem = objectItem;
    }

    object->length++;
    object->lastItem = objectItem;

    return VOXEL_OK_RET(objectItem);
}

VOXEL_ERRORABLE voxel_removeObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_Object* object = thing->value;
    voxel_ObjectItem* currentItem = object->firstItem;
    voxel_ObjectItem* previousItem = VOXEL_NULL;

    while (VOXEL_TRUE) {
        if (!currentItem) {
            return VOXEL_OK;
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

            VOXEL_MUST(voxel_unreferenceThing(context, currentItem->key));
            VOXEL_MUST(voxel_unreferenceThing(context, currentItem->value));

            VOXEL_FREE(currentItem); VOXEL_TAG_FREE(voxel_ObjectItem);

            object->length--;

            return VOXEL_OK;
        }

        previousItem = currentItem;
        currentItem = currentItem->nextItem;
    }
}

voxel_ObjectItemDescriptor* voxel_ensureObjectItemDescriptor(voxel_Context* context, voxel_ObjectItem* objectItem) {
    if (objectItem->descriptor) {
        return objectItem->descriptor;
    }

    voxel_ObjectItemDescriptor* descriptor = VOXEL_MALLOC(sizeof(voxel_ObjectItemDescriptor)); VOXEL_TAG_MALLOC(voxel_ObjectItemDescriptor);

    descriptor->getterFunction = VOXEL_NULL;
    descriptor->setterFunction = VOXEL_NULL;

    objectItem->descriptor = descriptor;

    return descriptor;
}

voxel_Count voxel_getObjectLength(voxel_Thing* thing) {
    voxel_Object* object = thing->value;

    // TODO: Maybe include lengths of prototypes as part of total returned length

    return object->length;
}

voxel_Thing* voxel_getObjectPrototypes(voxel_Context* context, voxel_Thing* thing) {
    voxel_Object* object = thing->value;

    if (object->prototypes) {
        return object->prototypes;
    }

    voxel_Thing* prototypesList = voxel_newList(context);

    if (thing->isLocked) {
        voxel_lockThing(prototypesList);
    }

    object->prototypes = prototypesList;
    prototypesList->referenceCount++;

    return prototypesList;
}

// src/lists.h

voxel_Thing* voxel_newList(voxel_Context* context) {
    voxel_List* list = VOXEL_MALLOC(sizeof(voxel_List)); VOXEL_TAG_MALLOC(voxel_List);

    list->length = 0;
    list->firstItem = VOXEL_NULL;
    list->lastItem = VOXEL_NULL;

    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_LIST);

    thing->type = VOXEL_TYPE_LIST;
    thing->value = list;

    return thing;
}

VOXEL_ERRORABLE voxel_destroyList(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_LIST);

    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;
    voxel_ListItem* nextItem;

    while (currentItem) {
        VOXEL_MUST(voxel_unreferenceThing(context, currentItem->value));

        nextItem = currentItem->nextItem;

        VOXEL_FREE(currentItem); VOXEL_TAG_FREE(voxel_ListItem);

        currentItem = nextItem;
    }

    VOXEL_FREE(list); VOXEL_TAG_FREE(voxel_List);
    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Bool voxel_compareLists(voxel_Thing* a, voxel_Thing* b) {
    voxel_List* aList = a->value;
    voxel_List* bList = b->value;

    if (aList->length != bList->length) {
        return VOXEL_FALSE;
    }

    voxel_ListItem* aCurrentItem = aList->firstItem;
    voxel_ListItem* bCurrentItem = bList->firstItem;

    while (VOXEL_TRUE) {
        if (!aCurrentItem && !bCurrentItem) {
            break;
        }

        if (!voxel_compareThings(aCurrentItem->value, bCurrentItem->value)) {
            return VOXEL_FALSE;
        }

        if (!aCurrentItem) {
            return VOXEL_FALSE;
        }

        if (!bCurrentItem) {
            return VOXEL_FALSE;
        }

        aCurrentItem = aCurrentItem->nextItem;
        bCurrentItem = bCurrentItem->nextItem;
    }

    return VOXEL_TRUE;
}

void voxel_lockList(voxel_Thing* thing) {
    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        voxel_lockThing(currentItem->value);

        currentItem = currentItem->nextItem;
    }
}

voxel_Thing* voxel_copyList(voxel_Context* context, voxel_Thing* thing) {
    // Shallow copy only; deep copying will be handled in the Voxel standard library

    voxel_List* list = thing->value;
    voxel_Thing* newList = voxel_newList(context);
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        voxel_pushOntoList(context, newList, currentItem->value);

        currentItem = currentItem->nextItem;
    }

    return newList;
}

VOXEL_ERRORABLE voxel_listToString(voxel_Context* context, voxel_Thing* thing) {
    voxel_Thing* delimeter = voxel_newStringTerminated(context, ",");
    VOXEL_ERRORABLE string = voxel_joinList(context, thing, delimeter);

    VOXEL_MUST(voxel_unreferenceThing(context, delimeter));

    return string;
}

VOXEL_ERRORABLE voxel_listToVxon(voxel_Context* context, voxel_Thing* thing) {
    voxel_List* list = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "[");
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        VOXEL_ERRORABLE itemString = voxel_thingToVxon(context, currentItem->value); VOXEL_MUST(itemString);

        VOXEL_MUST(voxel_appendToString(context, string, itemString.value));
        VOXEL_MUST(voxel_unreferenceThing(context, itemString.value));

        currentItem = currentItem->nextItem;

        if (currentItem) {
            VOXEL_MUST(voxel_appendByteToString(context, string, ','));
        }
    }

    VOXEL_MUST(voxel_appendByteToString(context, string, ']'));

    return VOXEL_OK_RET(string);
}

voxel_Bool voxel_listIsTruthy(voxel_Thing* thing) {
    voxel_List* list = thing->value;

    return list->length != 0;
}

VOXEL_ERRORABLE voxel_getListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index) {
    voxel_List* list = thing->value;
    voxel_ListItem* currentItem = list->firstItem;

    if (index >= list->length) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    if (index == list->length - 1) {
        return VOXEL_OK_RET(list->lastItem);
    }

    for (voxel_Count i = 0; i < index; i++) {
        if (!currentItem) {
            return VOXEL_OK_RET(VOXEL_NULL);
        }

        currentItem = currentItem->nextItem;
    }

    return VOXEL_OK_RET(currentItem);
}

VOXEL_ERRORABLE voxel_setListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARG);

    if (index == list->length) {
        return voxel_pushOntoList(context, thing, value);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* listItem = listItemResult.value;

    VOXEL_MUST(voxel_unreferenceThing(context, listItem->value));

    listItem->value = value;
    value->referenceCount++;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_removeListItem(voxel_Context* context, voxel_Thing* thing, voxel_Count index) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index < list->length, VOXEL_ERROR_INVALID_ARG);

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* listItem = listItemResult.value;

    if (listItem == list->firstItem) {
        list->firstItem = listItem->nextItem;
    }

    if (listItem == list->lastItem) {
        list->lastItem = listItem->previousItem;
    }

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = listItem->nextItem;
    }

    if (listItem->nextItem) {
        listItem->nextItem->previousItem = listItem->previousItem;
    }

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, listItem->value));
    VOXEL_FREE(listItem); VOXEL_TAG_FREE(voxel_ListItem);

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_pushOntoList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;
    voxel_ListItem* listItem = VOXEL_MALLOC(sizeof(voxel_ListItem)); VOXEL_TAG_MALLOC(voxel_ListItem);

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = list->lastItem;
    listItem->nextItem = VOXEL_NULL;

    if (list->lastItem) {
        list->lastItem->nextItem = listItem;
    }

    if (!list->firstItem) {
        list->firstItem = listItem;
    }

    list->length++;
    list->lastItem = listItem;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_popFromList(voxel_Context* context, voxel_Thing* thing) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);
    
    voxel_List* list = thing->value;
    voxel_ListItem* listItem = list->lastItem;

    if (!listItem) {
        return VOXEL_OK_RET(VOXEL_NULL);
    }

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = VOXEL_NULL;
    }

    list->lastItem = listItem->previousItem;

    if (listItem == list->firstItem) {
        list->firstItem = VOXEL_NULL;
    }

    voxel_Thing* value = listItem->value;

    list->length--;

    VOXEL_MUST(voxel_unreferenceThing(context, value));
    VOXEL_FREE(listItem); VOXEL_TAG_FREE(voxel_ListItem);

    return VOXEL_OK_RET(value);
}

VOXEL_ERRORABLE voxel_insertIntoList(voxel_Context* context, voxel_Thing* thing, voxel_Count index, voxel_Thing* value) {
    VOXEL_ASSERT(!thing->isLocked, VOXEL_ERROR_THING_LOCKED);

    voxel_List* list = thing->value;

    VOXEL_ASSERT(index >= 0, VOXEL_ERROR_INVALID_ARG);
    VOXEL_ASSERT(index <= list->length, VOXEL_ERROR_INVALID_ARG);

    if (index == list->length) {
        return voxel_pushOntoList(context, thing, value);
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(context, thing, index); VOXEL_MUST(listItemResult);
    voxel_ListItem* currentListItem = listItemResult.value;
    voxel_ListItem* listItem = VOXEL_MALLOC(sizeof(voxel_ListItem)); VOXEL_TAG_MALLOC(voxel_ListItem);

    listItem->value = value;
    value->referenceCount++;
    listItem->previousItem = currentListItem->previousItem;
    listItem->nextItem = currentListItem;
    currentListItem->previousItem = listItem;

    if (listItem->previousItem) {
        listItem->previousItem->nextItem = listItem;
    }

    if (currentListItem == list->firstItem) {
        list->firstItem = listItem;
    }

    list->length++;

    return VOXEL_OK;
}

voxel_Count voxel_getListLength(voxel_Thing* thing) {
    voxel_List* list = thing->value;

    return list->length;
}

VOXEL_ERRORABLE voxel_joinList(voxel_Context* context, voxel_Thing* thing, voxel_Thing* delimeter) {
    voxel_List* list = thing->value;
    voxel_Thing* string = voxel_newStringTerminated(context, "");
    voxel_ListItem* currentItem = list->firstItem;

    while (currentItem) {
        VOXEL_ERRORABLE itemString = voxel_thingToString(context, currentItem->value); VOXEL_MUST(itemString);

        VOXEL_MUST(voxel_appendToString(context, string, itemString.value));
        VOXEL_MUST(voxel_unreferenceThing(context, itemString.value));

        currentItem = currentItem->nextItem;

        if (delimeter && currentItem) {
            VOXEL_MUST(voxel_appendToString(context, string, delimeter));
        }
    }

    return VOXEL_OK_RET(string);
}

// src/operations.h

VOXEL_ERRORABLE voxel_notOperation(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newBoolean(context, !voxel_thingIsTruthy(thing)));
}

VOXEL_ERRORABLE voxel_andOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    if (a->type == VOXEL_TYPE_BYTE) {
        VOXEL_ERRORABLE bByteResult = voxel_thingToByte(context, b); VOXEL_MUST(bByteResult);
        voxel_Thing* bByte = bByteResult.value;
        voxel_Thing* result = voxel_newByte(context, (voxel_IntPtr)a->value & (voxel_IntPtr)bByte->value);

        VOXEL_MUST(voxel_unreferenceThing(context, bByte));

        return VOXEL_OK_RET(result);
    }

    if (a->type == VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);
        voxel_Thing* bNumber = bNumberResult.value;

        VOXEL_MUST(voxel_unreferenceThing(context, bNumber));

        return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getNumberInt(a) & voxel_getNumberInt(bNumber)));
    }

    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) && voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_orOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    if (a->type == VOXEL_TYPE_BYTE) {
        VOXEL_ERRORABLE bByteResult = voxel_thingToByte(context, b); VOXEL_MUST(bByteResult);
        voxel_Thing* bByte = bByteResult.value;
        voxel_Thing* result = voxel_newByte(context, (voxel_IntPtr)a->value | (voxel_IntPtr)bByte->value);

        VOXEL_MUST(voxel_unreferenceThing(context, bByte));

        return VOXEL_OK_RET(result);
    }

    if (a->type == VOXEL_TYPE_NUMBER) {
        VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);
        voxel_Thing* bNumber = bNumberResult.value;

        VOXEL_MUST(voxel_unreferenceThing(context, bNumber));

        return VOXEL_OK_RET(voxel_newNumberInt(context, voxel_getNumberInt(a) | voxel_getNumberInt(bNumber)));
    }

    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_thingIsTruthy(a) || voxel_thingIsTruthy(b)));
}

VOXEL_ERRORABLE voxel_equalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    return VOXEL_OK_RET(voxel_newBoolean(context, voxel_compareThings(a, b)));
}

VOXEL_ERRORABLE voxel_lessThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat(aNumberResult.value) < voxel_getNumberFloat(bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}

VOXEL_ERRORABLE voxel_greaterThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b) {
    VOXEL_ERRORABLE aNumberResult = voxel_thingToNumber(context, a); VOXEL_MUST(aNumberResult);
    VOXEL_ERRORABLE bNumberResult = voxel_thingToNumber(context, b); VOXEL_MUST(bNumberResult);

    voxel_Bool result = voxel_getNumberFloat(aNumberResult.value) > voxel_getNumberFloat(bNumberResult.value);

    VOXEL_MUST(voxel_unreferenceThing(context, aNumberResult.value));
    VOXEL_MUST(voxel_unreferenceThing(context, bNumberResult.value));

    return VOXEL_OK_RET(voxel_newBoolean(context, result));
}

// src/parser.h

VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Position* position, voxel_Count bytesToRead) {
    if ((*position) + bytesToRead > context->codeLength) {
        VOXEL_THROW(VOXEL_ERROR_TOKENISATION_END);
    }

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_nextToken(voxel_Context* context, voxel_Position* position) {
    VOXEL_ASSERT(context->code, VOXEL_ERROR_NO_CODE);
    
    voxel_Token token;
    voxel_Byte tokenType;

    switch (context->tokenisationState) {
        case VOXEL_STATE_SYSTEM_CALL_GET:
            tokenType = VOXEL_TOKEN_TYPE_GET;
            context->tokenisationState = VOXEL_STATE_SYSTEM_CALL_CALL;
            break;

        case VOXEL_STATE_SYSTEM_CALL_CALL:
            tokenType = VOXEL_TOKEN_TYPE_CALL;
            context->tokenisationState = VOXEL_STATE_NONE;
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
                bufferSize |= context->code[(*position)++] & 0xFF;
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
        case VOXEL_TOKEN_TYPE_SYSTEM_CALL:
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
                    currentString = VOXEL_MALLOC(neededSize); VOXEL_TAG_MALLOC_SIZE("voxel_Byte[]", neededSize);
                    currentSize = neededSize;
                } else if (neededSize > currentSize) {
                    currentString = VOXEL_REALLOC(currentString, neededSize); VOXEL_TAG_REALLOC("voxel_Byte[]", currentSize, neededSize);
                    currentSize = neededSize;
                }

                currentString[stringSize++] = currentByte;
            }

            token.data = voxel_newString(context, stringSize, currentString);

            VOXEL_FREE(currentString); VOXEL_TAG_FREE_SIZE("voxel_Byte[]", currentSize);

            VOXEL_DEBUG_LOG("[Token: string]\n");

            if (tokenType == VOXEL_TOKEN_TYPE_SYSTEM_CALL) {
                context->tokenisationState = VOXEL_STATE_SYSTEM_CALL_GET;
            }

            break;

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
        case VOXEL_TOKEN_TYPE_SWAP:
        case VOXEL_TOKEN_TYPE_POS_REF_HERE:
        case VOXEL_TOKEN_TYPE_JUMP:
        case VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY:
        case VOXEL_TOKEN_TYPE_EQUAL:
        case VOXEL_TOKEN_TYPE_LESS_THAN:
        case VOXEL_TOKEN_TYPE_GREATER_THAN:
        case VOXEL_TOKEN_TYPE_NOT:
        case VOXEL_TOKEN_TYPE_AND:
        case VOXEL_TOKEN_TYPE_OR:
            VOXEL_DEBUG_LOG("[Non-thing token]\n");
            break;

        case VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE:
        case VOXEL_TOKEN_TYPE_POS_REF_BACKWARD:
        case VOXEL_TOKEN_TYPE_POS_REF_FORWARD:
            VOXEL_MUST(voxel_safeToRead(context, position, 4));

            voxel_UInt32 stepSize = 0;

            for (voxel_Count i = 0; i < 4; i++) {
                stepSize <<= 8;
                stepSize |= context->code[(*position)++] & 0xFF;
            }

            token.data = (void*)(voxel_IntPtr)stepSize;

            VOXEL_DEBUG_LOG("[Token: position reference]\n");

            break;

        case '\0':
            VOXEL_DEBUG_LOG("[Last byte]\n");

            return VOXEL_OK_RET(VOXEL_NULL);

        default:
            VOXEL_THROW(VOXEL_ERROR_TOKENISATION_BYTE);
    }

    token.type = tokenType;

    voxel_Token* tokenPtr = VOXEL_MALLOC(sizeof(token)); VOXEL_TAG_MALLOC(voxel_Token);

    VOXEL_INTO_PTR(token, tokenPtr);

    return VOXEL_OK_RET(tokenPtr);
}

// src/executors.h

voxel_Executor* voxel_newExecutor(voxel_Context* context) {
    voxel_Executor* executor = VOXEL_MALLOC(sizeof(voxel_Executor)); VOXEL_TAG_MALLOC(voxel_Executor);

    executor->context = context;
    executor->scope = context->globalScope;
    executor->isRunning = VOXEL_TRUE;
    executor->callStackSize = VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call);
    executor->callStack = VOXEL_MALLOC(executor->callStackSize); VOXEL_TAG_MALLOC_SIZE("executor->callStack", VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call));
    executor->callStack[0] = (voxel_Call) {.position = VOXEL_MAGIC_SIZE, .canHandleExceptions = VOXEL_FALSE};
    executor->callStackHead = 0;
    executor->valueStack = voxel_newList(context);
    executor->previousExecutor = context->lastExecutor;
    executor->nextExecutor = VOXEL_NULL;

    if (!context->firstExecutor) {
        context->firstExecutor = executor;
    }
    
    if (context->lastExecutor) {
        context->lastExecutor->nextExecutor = executor;
    }

    context->lastExecutor = executor;

    return executor;
}

voxel_Position* voxel_getExecutorPosition(voxel_Executor* executor) {
    return &executor->callStack[executor->callStackHead].position;
}

VOXEL_ERRORABLE voxel_stepExecutor(voxel_Executor* executor) {
    VOXEL_ASSERT(executor->context->isInitialised, VOXEL_ERROR_NOT_INITIALISED);

    if (!executor->isRunning) {
        return VOXEL_OK;
    }

    voxel_Position* position = voxel_getExecutorPosition(executor);
    VOXEL_ERRORABLE tokenResult = voxel_nextToken(executor->context, position); VOXEL_MUST(tokenResult);
    voxel_Token* token = tokenResult.value;

    if (!token) {
        executor->isRunning = VOXEL_FALSE;

        return VOXEL_OK;
    }

    switch (token->type) {
        case VOXEL_TOKEN_TYPE_CALL:
            VOXEL_ERRORABLE callThingResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(callThingResult);
            voxel_Thing* callThing = callThingResult.value;

            VOXEL_ASSERT(callThing, VOXEL_ERROR_MISSING_ARG);

            if (callThing->type == VOXEL_TYPE_FUNCTION) {
                voxel_FunctionType functionType = voxel_getFunctionType(executor->context, callThing);

                if (functionType == VOXEL_FUNCTION_TYPE_BUILTIN) {
                    voxel_Count builtinFunctionIndex = (voxel_IntPtr)callThing->value;

                    builtinFunctionIndex *= -1;
                    builtinFunctionIndex--;

                    VOXEL_ASSERT(
                        builtinFunctionIndex >= 0 && builtinFunctionIndex < executor->context->builtinCount,
                        VOXEL_ERROR_INVALID_BUILTIN
                    );

                    voxel_Builtin builtin = executor->context->builtins[builtinFunctionIndex];

                    (*builtin)(executor);

                    break;
                }

                voxel_stepInExecutor(executor, (voxel_Position)(voxel_IntPtr)callThing->value);
            } else if (callThing->type == VOXEL_TYPE_CLOSURE) {
                voxel_Closure* closure = callThing->value;

                voxel_stepInExecutor(executor, closure->position);

                voxel_Scope* scope = executor->scope;
                voxel_Thing* environment = closure->environment;
                voxel_Object* environmentObject = environment->value;
                voxel_ObjectItem* currentItem = environmentObject->firstItem;

                while (currentItem) {
                    voxel_setObjectItem(executor->context, scope->things, currentItem->key, currentItem->value);

                    currentItem = currentItem->nextItem;
                }
            } else {
                VOXEL_THROW(VOXEL_ERROR_CANNOT_CALL_THING);
            }

            VOXEL_MUST(voxel_unreferenceThing(executor->context, callThing));

            break;

        case VOXEL_TOKEN_TYPE_RETURN:
            voxel_stepOutExecutor(executor);
            break;

        case VOXEL_TOKEN_TYPE_THROW:
            VOXEL_MUST(voxel_throwException(executor));
            break;

        case VOXEL_TOKEN_TYPE_SET_HANDLER:
            VOXEL_ERRORABLE handlerFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(handlerFunctionResult);
            voxel_Thing* handlerFunction = handlerFunctionResult.value;

            VOXEL_ASSERT(handlerFunction, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(handlerFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(voxel_getFunctionType(executor->context, handlerFunction) == VOXEL_FUNCTION_TYPE_POS_REF, VOXEL_ERROR_INVALID_ARG);

            voxel_setExceptionHandler(executor, (voxel_IntPtr)handlerFunction->value);

            VOXEL_MUST(voxel_unreferenceThing(executor->context, handlerFunction));

            break;

        case VOXEL_TOKEN_TYPE_CLEAR_HANDLER:
            voxel_clearExceptionHandler(executor);
            break;

        case VOXEL_TOKEN_TYPE_GET:
            VOXEL_ERRORABLE getKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(getKey);

            VOXEL_ASSERT(getKey.value, VOXEL_ERROR_MISSING_ARG);

            voxel_ObjectItem* scopeItem = voxel_getScopeItem(executor->scope, getKey.value);
            voxel_Thing* scopeValue = scopeItem ? scopeItem->value : voxel_newNull(executor->context);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, scopeValue));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, getKey.value));

            scopeValue->referenceCount++; // To ensure that builtins don't dereference the thing in the scope

            break;

        case VOXEL_TOKEN_TYPE_SET:
        case VOXEL_TOKEN_TYPE_VAR:
            VOXEL_ERRORABLE setKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(setKey);
            voxel_Thing* setValue = ((voxel_List*)executor->valueStack->value)->lastItem->value;

            VOXEL_ASSERT(setKey.value, VOXEL_ERROR_MISSING_ARG);
            VOXEL_ASSERT(setValue, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST((token->type == VOXEL_TOKEN_TYPE_VAR ? voxel_setLocalScopeItem : voxel_setScopeItem)(executor->scope, setKey.value, setValue));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, setKey.value));

            break;

        case VOXEL_TOKEN_TYPE_POP:
            VOXEL_ERRORABLE popResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(popResult);

            VOXEL_ASSERT(popResult.value, VOXEL_ERROR_MISSING_ARG);

            VOXEL_MUST(voxel_unreferenceThing(executor->context, popResult.value));

            break;

        case VOXEL_TOKEN_TYPE_DUPE:
            voxel_Count dupeStackLength = voxel_getListLength(executor->valueStack);

            VOXEL_ASSERT(dupeStackLength > 0, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE dupeListItemResult = voxel_getListItem(executor->context, executor->valueStack, dupeStackLength - 1); VOXEL_MUST(dupeListItemResult);
            voxel_ListItem* dupeListItem = dupeListItemResult.value;
            voxel_Thing* dupeThing = dupeListItem->value;

            dupeThing->referenceCount++;

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, dupeThing));

            break;

        case VOXEL_TOKEN_TYPE_SWAP:
            VOXEL_ERRORABLE swapBResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(swapBResult);
            VOXEL_ERRORABLE swapAResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(swapAResult);

            VOXEL_ASSERT(swapAResult.value, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(swapBResult.value, VOXEL_ERROR_INVALID_ARG);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, swapBResult.value));
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, swapAResult.value));

            break;

        case VOXEL_TOKEN_TYPE_POS_REF_HERE:
        case VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE:
        case VOXEL_TOKEN_TYPE_POS_REF_BACKWARD:
        case VOXEL_TOKEN_TYPE_POS_REF_FORWARD:
            VOXEL_ERRORABLE posRefKey = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(posRefKey);
            voxel_Position referencedPosition = *position;

            VOXEL_ASSERT(posRefKey.value, VOXEL_ERROR_INVALID_ARG);

            if (token->type == VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE) {
                referencedPosition = (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_BACKWARD) {
                referencedPosition -= (voxel_IntPtr)token->data;
            } else if (token->type == VOXEL_TOKEN_TYPE_POS_REF_FORWARD) {
                referencedPosition += (voxel_IntPtr)token->data;
            }

            voxel_Thing* function = voxel_newFunctionPosRef(executor->context, referencedPosition);

            VOXEL_MUST(voxel_setScopeItem(executor->scope, posRefKey.value, function));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, posRefKey.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, function));

            break;

        case VOXEL_TOKEN_TYPE_JUMP:
        case VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY:
            VOXEL_ERRORABLE jumpFunctionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpFunctionResult);
            voxel_Thing* jumpFunction = jumpFunctionResult.value;

            VOXEL_ASSERT(jumpFunction, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(jumpFunction->type == VOXEL_TYPE_FUNCTION, VOXEL_ERROR_CANNOT_JUMP_TO_THING);
            VOXEL_ASSERT(voxel_getFunctionType(executor->context, jumpFunction) == VOXEL_FUNCTION_TYPE_POS_REF, VOXEL_ERROR_CANNOT_JUMP_TO_THING);

            if (token->type == VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY) {
                VOXEL_ERRORABLE jumpConditionResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(jumpConditionResult);
                voxel_Thing* jumpCondition = jumpConditionResult.value;

                if (jumpCondition) {
                    voxel_Bool truthiness = voxel_thingIsTruthy(jumpCondition);

                    VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpCondition));

                    if (!truthiness) {
                        VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

                        break;
                    }
                }
            }

            *position = (voxel_IntPtr)jumpFunction->value;

            VOXEL_MUST(voxel_unreferenceThing(executor->context, jumpFunction));

            break;

        case VOXEL_TOKEN_TYPE_NOT:
            VOXEL_ERRORABLE notValueResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(notValueResult);

            VOXEL_ASSERT(notValueResult.value, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE notResult = voxel_notOperation(executor->context, notValueResult.value); VOXEL_MUST(notResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, notResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, notValueResult.value));

            break;

        case VOXEL_TOKEN_TYPE_AND:
        case VOXEL_TOKEN_TYPE_OR:
        case VOXEL_TOKEN_TYPE_EQUAL:
        case VOXEL_TOKEN_TYPE_LESS_THAN:
        case VOXEL_TOKEN_TYPE_GREATER_THAN:
            VOXEL_ERRORABLE binaryBResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryBResult);
            VOXEL_ERRORABLE binaryAResult = voxel_popFromList(executor->context, executor->valueStack); VOXEL_MUST(binaryAResult);

            VOXEL_ASSERT(binaryAResult.value, VOXEL_ERROR_INVALID_ARG);
            VOXEL_ASSERT(binaryBResult.value, VOXEL_ERROR_INVALID_ARG);

            VOXEL_ERRORABLE binaryResult;

            if (token->type == VOXEL_TOKEN_TYPE_AND) {
                binaryResult = voxel_andOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_OR) {
                binaryResult = voxel_orOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_EQUAL) {
                binaryResult = voxel_equalOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_LESS_THAN) {
                binaryResult = voxel_lessThanOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else if (token->type == VOXEL_TOKEN_TYPE_GREATER_THAN) {
                binaryResult = voxel_greaterThanOperation(executor->context, binaryAResult.value, binaryBResult.value);
            } else {
                VOXEL_THROW(VOXEL_ERROR_NOT_IMPLEMENTED);
            }

            VOXEL_MUST(binaryResult);

            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, binaryResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryAResult.value));
            VOXEL_MUST(voxel_unreferenceThing(executor->context, binaryBResult.value));

            break;

        default:
            // Token contains thing to be pushed onto value stack
            VOXEL_MUST(voxel_pushOntoList(executor->context, executor->valueStack, token->data));
            break;
    }

    #ifdef VOXEL_DEBUG_EXECUTORS
        VOXEL_LOG("Current value stack: ");
        voxel_logThing(executor->context, executor->valueStack);
        VOXEL_LOG("\n");
    #endif

    VOXEL_FREE(token); VOXEL_TAG_FREE(voxel_Token);

    return VOXEL_OK;
}

void voxel_stepInExecutor(voxel_Executor* executor, voxel_Position position) {
    voxel_Scope* newScope = voxel_newScope(executor->context, executor->scope);

    executor->scope = newScope;

    executor->callStackHead++;

    voxel_Count neededSize = ((executor->callStackHead / VOXEL_CALL_STACK_BLOCK_LENGTH) + 1) * VOXEL_CALL_STACK_BLOCK_LENGTH * sizeof(voxel_Call);

    if (executor->callStackSize < neededSize) {
        executor->callStackSize = neededSize;
        executor->callStack = VOXEL_REALLOC(executor->callStack, neededSize); VOXEL_TAG_REALLOC("voxel_Executor->callStack", neededSize - sizeof(voxel_Call), neededSize);
    }

    executor->callStack[executor->callStackHead] = (voxel_Call) {.position = position, .canHandleExceptions = VOXEL_FALSE};
}

void voxel_stepOutExecutor(voxel_Executor* executor) {
    if (executor->callStackHead == 0) {
        executor->isRunning = VOXEL_FALSE;

        return;
    }

    voxel_Scope* parentScope = executor->scope->parentScope;

    if (parentScope) {
        voxel_destroyScope(executor->scope);

        executor->scope = parentScope;
    }

    executor->callStackHead--;
}

void voxel_setExceptionHandler(voxel_Executor* executor, voxel_Position exceptionHandlerPosition) {
    voxel_Call* callStackTop = &executor->callStack[executor->callStackHead];

    callStackTop->canHandleExceptions = VOXEL_TRUE;
    callStackTop->exceptionHandlerPosition = exceptionHandlerPosition;
}

void voxel_clearExceptionHandler(voxel_Executor* executor) {
    executor->callStack[executor->callStackHead].canHandleExceptions = VOXEL_FALSE;
}

VOXEL_ERRORABLE voxel_throwException(voxel_Executor* executor) {
    while (!executor->callStack[executor->callStackHead].canHandleExceptions) {
        if (executor->callStackHead == 0) {
            #ifdef VOXEL_LOG_UNHANDLED_EXCEPTIONS
                voxel_Thing* valueStack = executor->valueStack;
                voxel_List* valueStackList = valueStack->value;
                voxel_ListItem* lastItem = valueStackList->lastItem;

                if (lastItem) {
                    VOXEL_LOG("Unhandled exception: ");
                    VOXEL_MUST(voxel_logThing(executor->context, lastItem->value));
                    VOXEL_LOG("\n");
                }
            #endif

            VOXEL_THROW(VOXEL_ERROR_UNHANDLED_EXCEPTION);
        }

        voxel_stepOutExecutor(executor);
    }

    *voxel_getExecutorPosition(executor) = executor->callStack[executor->callStackHead].exceptionHandlerPosition;

    voxel_clearExceptionHandler(executor);

    return VOXEL_OK;
}

// src/scopes.h

voxel_Scope* voxel_newScope(voxel_Context* context, voxel_Scope* parentScope) {
    voxel_Scope* scope = VOXEL_MALLOC(sizeof(voxel_Scope)); VOXEL_TAG_MALLOC(voxel_Scope);

    scope->context = context;
    scope->parentScope = parentScope;
    scope->things = voxel_newObject(context);

    return scope;
}

VOXEL_ERRORABLE voxel_destroyScope(voxel_Scope* scope) {
    VOXEL_MUST(voxel_unreferenceThing(scope->context, scope->things));

    VOXEL_FREE(scope); VOXEL_TAG_FREE(voxel_Scope);
}

voxel_ObjectItem* voxel_getScopeItem(voxel_Scope* scope, voxel_Thing* key) {
    voxel_ObjectItem* thisScopeItem = voxel_getObjectItem(scope->things, key);

    if (thisScopeItem) {
        return thisScopeItem;
    }

    if (scope->parentScope) {
        return voxel_getScopeItem(scope->context->globalScope, key);
    }

    return VOXEL_NULL;
}

VOXEL_ERRORABLE voxel_setScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value) {
    voxel_ObjectItem* scopeItem = voxel_getScopeItem(scope, key);

    if (!scopeItem) {
        return voxel_setObjectItem(scope->context, scope->things, key, value);
    }

    voxel_unreferenceThing(scope->context, scopeItem->value);

    scopeItem->value = value;
    value->referenceCount++;

    return VOXEL_OK;
}

VOXEL_ERRORABLE voxel_setLocalScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value) {
    return voxel_setObjectItem(scope->context, scope->things, key, value);
}

// src/helpers.h

#define _VOXEL_HELPER_POP_VALUE(name, type, popCall, getValueCall, defaultValue) type name(voxel_Executor* executor) { \
        voxel_Thing* thing = popCall(executor); \
\
        if (!thing) { \
            return (defaultValue); \
        } \
\
        type result = getValueCall(thing); \
\
        voxel_unreferenceThing(executor->context, thing); \
\
        return result; \
    }

void voxel_push(voxel_Executor* executor, voxel_Thing* thing) {
    if (thing == VOXEL_NULL) {
        thing = voxel_newNull(executor->context);
    }

    voxel_pushOntoList(executor->context, executor->valueStack, thing);
}

void voxel_pushNull(voxel_Executor* executor) {
    voxel_push(executor, voxel_newNull(executor->context));
}

voxel_Thing* voxel_pop(voxel_Executor* executor) {
    VOXEL_ERRORABLE result = voxel_popFromList(executor->context, executor->valueStack);

    if (VOXEL_IS_ERROR(result)) {
        return VOXEL_NULL;
    }

    return result.value;
}

void voxel_popVoid(voxel_Executor* executor) {
    voxel_unreferenceThing(executor->context, voxel_pop(executor));
}

voxel_Thing* voxel_popNumber(voxel_Executor* executor) {
    voxel_Thing* poppedThing = voxel_pop(executor);

    if (!poppedThing) {
        return VOXEL_NULL;
    }

    if (poppedThing->type == VOXEL_TYPE_NUMBER) {
        return poppedThing; // Saves us from having to copy the thing
    }

    VOXEL_ERRORABLE result = voxel_thingToNumber(executor->context, poppedThing);

    voxel_unreferenceThing(executor->context, poppedThing);

    if (VOXEL_IS_ERROR(result)) {
        return VOXEL_NULL;
    }

    return result.value;
}

_VOXEL_HELPER_POP_VALUE(voxel_popNumberInt, voxel_Int, voxel_popNumber, voxel_getNumberInt, 0);
_VOXEL_HELPER_POP_VALUE(voxel_popNumberFloat, voxel_Float, voxel_popNumber, voxel_getNumberFloat, 0);

voxel_Thing* voxel_popString(voxel_Executor* executor) {
    voxel_Thing* poppedThing = voxel_pop(executor);

    if (!poppedThing) {
        return VOXEL_NULL;
    }

    if (poppedThing->type == VOXEL_TYPE_STRING) {
        return poppedThing; // Saves us from having to copy the thing
    }

    VOXEL_ERRORABLE result = voxel_thingToString(executor->context, poppedThing);

    voxel_unreferenceThing(executor->context, poppedThing);

    if (VOXEL_IS_ERROR(result)) {
        return VOXEL_NULL;
    }

    return result.value;
}

voxel_Thing* voxel_peek(voxel_Executor* executor, voxel_Int index) {
    voxel_Thing* stack = executor->valueStack;

    index = voxel_getListLength(stack) - index - 1;

    if (index < 0) {
        return VOXEL_NULL;
    }

    VOXEL_ERRORABLE listItemResult = voxel_getListItem(executor->context, stack, index);

    if (VOXEL_IS_ERROR(listItemResult)) {
        return VOXEL_NULL;
    }

    voxel_ListItem* listItem = listItemResult.value;

    return listItem->value;
}

// src/voxel.h

void voxel_test() {
    VOXEL_LOG("Hello from Voxel!\n");
}

#endif
