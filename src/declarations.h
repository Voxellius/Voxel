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
    voxel_Count codeSize;
    voxel_Builtin* builtins;
    voxel_Count builtinCount;
    struct voxel_Scope* globalScope;
    struct voxel_Thing* firstTrackedThing;
    struct voxel_Thing* lastTrackedThing;
    voxel_Count executorCount;
    struct voxel_Executor* firstExecutor;
    struct voxel_Executor* lastExecutor;
    struct voxel_Thing* weakRefs;
    struct voxel_Thing* enumLookup;
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
    VOXEL_TYPE_LIST,
    VOXEL_TYPE_WEAK
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
    VOXEL_TOKEN_TYPE_THIS = 'M',
    VOXEL_TOKEN_TYPE_SET_HANDLER = 'H',
    VOXEL_TOKEN_TYPE_CLEAR_HANDLER = 'h',
    VOXEL_TOKEN_TYPE_GET = '?',
    VOXEL_TOKEN_TYPE_SET = ':',
    VOXEL_TOKEN_TYPE_VAR = 'v',
    VOXEL_TOKEN_TYPE_DELETE = 'D',
    VOXEL_TOKEN_TYPE_POP = 'p',
    VOXEL_TOKEN_TYPE_DUPE = 'd',
    VOXEL_TOKEN_TYPE_OVER = 'o',
    VOXEL_TOKEN_TYPE_SWAP = 's',
    VOXEL_TOKEN_TYPE_COPY = 'c',
    VOXEL_TOKEN_TYPE_POS_REF_HERE = '@',
    VOXEL_TOKEN_TYPE_POS_REF_ABSOLUTE = '#',
    VOXEL_TOKEN_TYPE_POS_REF_BACKWARD = '[',
    VOXEL_TOKEN_TYPE_POS_REF_FORWARD = ']',
    VOXEL_TOKEN_TYPE_JUMP = 'J',
    VOXEL_TOKEN_TYPE_JUMP_IF_TRUTHY = 'I',
    VOXEL_TOKEN_TYPE_IDENTICAL = 'i',
    VOXEL_TOKEN_TYPE_EQUAL = '=',
    VOXEL_TOKEN_TYPE_LESS_THAN = '<',
    VOXEL_TOKEN_TYPE_GREATER_THAN = '>',
    VOXEL_TOKEN_TYPE_NOT = 'N',
    VOXEL_TOKEN_TYPE_AND = 'A',
    VOXEL_TOKEN_TYPE_OR = 'O',
    VOXEL_TOKEN_TYPE_ENUM_LOOKUP_REGISTER = 'e'
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
    voxel_Thing* preserveSymbols;
    voxel_Count id;
    voxel_Bool isRunning;
    voxel_Bool hasFinished;
    voxel_TokenisationState tokenisationState;
    voxel_Call* callStack;
    voxel_Count callStackHead;
    voxel_Count callStackSize;
    voxel_Thing* valueStack;
    voxel_Thing* thisStack;
    voxel_Thing* nextThis;
    voxel_Thing* superStack;
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
voxel_Bool voxel_anyExecutorsRunning(voxel_Context* context);
VOXEL_ERRORABLE voxel_defineBuiltin(voxel_Context* context, voxel_Byte* name, voxel_Builtin builtin);

voxel_Thing* voxel_newThing(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyThing(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_unreferenceThing(voxel_Context* context, voxel_Thing* thing);
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
voxel_Thing* voxel_stringToBuffer(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_stringToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_stringIsTruthy(voxel_Thing* thing);
voxel_Count voxel_getStringSize(voxel_Thing* thing);
voxel_Count voxel_stringCharIndexToByteIndex(voxel_Thing* thing, voxel_Count charIndex);
voxel_Count voxel_getStringLength(voxel_Thing* thing);
voxel_Thing* voxel_getStringByteRange(voxel_Context* context, voxel_Thing* thing, voxel_Count start, voxel_Count end);
void voxel_logString(voxel_Thing* thing);
voxel_Thing* voxel_concatenateStrings(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_appendToString(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_appendToStringTerminatedBytes(voxel_Context* context, voxel_Thing* a, voxel_Byte* b);
VOXEL_ERRORABLE voxel_appendByteToString(voxel_Context* context, voxel_Thing* thing, voxel_Byte byte);
VOXEL_ERRORABLE voxel_reverseString(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_cutStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count size);
VOXEL_ERRORABLE voxel_cutStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count size);
VOXEL_ERRORABLE voxel_padStringStart(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Thing* fill);
VOXEL_ERRORABLE voxel_padStringEnd(voxel_Context* context, voxel_Thing* thing, voxel_Count minSize, voxel_Thing* fill);

voxel_Thing* voxel_newObject(voxel_Context* context);
VOXEL_ERRORABLE voxel_destroyObject(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_compareObjects(voxel_Thing* a, voxel_Thing* b);
void voxel_lockObject(voxel_Thing* thing);
voxel_Thing* voxel_copyObject(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_objectToVxon(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_objectIsTruthy(voxel_Thing* thing);
voxel_ObjectItem* voxel_getPrototypedObjectItem(voxel_Thing* thing, voxel_Thing* key, voxel_Count traverseDepth, voxel_Count* actualTraverseDepth, voxel_Thing** actualParentObject);
voxel_ObjectItem* voxel_getObjectItem(voxel_Thing* thing, voxel_Thing* key);
VOXEL_ERRORABLE voxel_setObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key, voxel_Thing* value);
VOXEL_ERRORABLE voxel_removeObjectItem(voxel_Context* context, voxel_Thing* thing, voxel_Thing* key);
voxel_ObjectItemDescriptor* voxel_ensureObjectItemDescriptor(voxel_Context* context, voxel_ObjectItem* objectItem);
VOXEL_ERRORABLE voxel_addPrototypedObjectKeys(voxel_Context* context, voxel_Thing* thing, voxel_Thing* list, voxel_Count traverseDepth);
VOXEL_ERRORABLE voxel_getObjectKeys(voxel_Context* context, voxel_Thing* thing, voxel_Count traverseDepth);
voxel_Count voxel_getObjectLength(voxel_Thing* thing);
voxel_Thing* voxel_getObjectPrototypes(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_checkWhetherObjectInherits(voxel_Thing* thing, voxel_Thing* target, voxel_Count traverseDepth);

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
VOXEL_ERRORABLE voxel_concatList(voxel_Context* context, voxel_Thing* destination, voxel_Thing* source);

voxel_Thing* voxel_newWeakRef(voxel_Context* context, voxel_Thing* target);
VOXEL_ERRORABLE voxel_destroyWeakRef(voxel_Context* context, voxel_Thing* thing);
voxel_Thing* voxel_dereferenceWeakRef(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_compareWeakRefs(voxel_Thing* a, voxel_Thing* b);
voxel_Thing* voxel_copyWeakRef(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_weakRefToString(voxel_Context* context, voxel_Thing* thing);
voxel_Bool voxel_weakRefIsTruthy(voxel_Thing* thing);
void voxel_unreferenceFromWeakRefs(voxel_Context* context, voxel_Thing* thing);

VOXEL_ERRORABLE voxel_registerEnumEntry(voxel_Context* context, voxel_Thing* value, voxel_Thing* identifier);
voxel_Thing* voxel_getEnumEntryFromLookup(voxel_Context* context, voxel_Thing* value);

VOXEL_ERRORABLE voxel_notOperation(voxel_Context* context, voxel_Thing* thing);
VOXEL_ERRORABLE voxel_andOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_orOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_identicalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_equalOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_lessThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);
VOXEL_ERRORABLE voxel_greaterThanOperation(voxel_Context* context, voxel_Thing* a, voxel_Thing* b);

VOXEL_ERRORABLE voxel_safeToRead(voxel_Context* context, voxel_Position* position, voxel_Count bytesToRead);
VOXEL_ERRORABLE voxel_nextToken(voxel_Executor* executor, voxel_Position* position);

voxel_Scope* voxel_newScope(voxel_Context* context, voxel_Scope* parentScope);
VOXEL_ERRORABLE voxel_destroyScope(voxel_Scope* scope);
voxel_ObjectItem* voxel_getScopeItem(voxel_Scope* scope, voxel_Thing* key);
VOXEL_ERRORABLE voxel_setScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value);
VOXEL_ERRORABLE voxel_setLocalScopeItem(voxel_Scope* scope, voxel_Thing* key, voxel_Thing* value);
VOXEL_ERRORABLE voxel_removeScopeItem(voxel_Scope* scope, voxel_Thing* key);

voxel_Executor* voxel_newExecutor(voxel_Context* context);
voxel_Executor* voxel_cloneExecutor(voxel_Executor* executor, voxel_Bool copyValueStack);
VOXEL_ERRORABLE voxel_destroyExecutor(voxel_Executor* executor);
voxel_Executor* voxel_getExecutorById(voxel_Context* context, voxel_Count id);
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
voxel_Bool voxel_popBoolean(voxel_Executor* executor);
voxel_Thing* voxel_popByte(voxel_Executor* executor);
voxel_Thing* voxel_popNumber(voxel_Executor* executor);
voxel_Int voxel_popNumberInt(voxel_Executor* executor);
voxel_Float voxel_popNumberFloat(voxel_Executor* executor);
voxel_Thing* voxel_popString(voxel_Executor* executor);
voxel_Thing* voxel_peek(voxel_Executor* executor, voxel_Int index);

void voxel_test();