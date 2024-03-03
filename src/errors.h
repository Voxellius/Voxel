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
#define VOXEL_ERROR_INVALID_ARGUMENT -0x22
#define VOXEL_ERROR_THING_LOCKED -0x30
#define VOXEL_ERROR_CANNOT_CONVERT_THING -0x31
#define VOXEL_ERROR_NOT_A_MEMBER -0x32
#define VOXEL_ERROR_CANNOT_CALL_THING -0x33
#define VOXEL_ERROR_CANNOT_JUMP_TO_THING -0x34
#define VOXEL_ERROR_INVALID_BUILTIN -0x35
#define VOXEL_ERROR_MISSING_ARG -0x40

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

        case VOXEL_ERROR_INVALID_ARGUMENT:
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

        default:
            return "Unknown error";
    }
}