#define VOXEL_ERROR_CODE int

#define VOXEL_ERRORABLE VOXEL_ERROR_CODE

#define VOXEL_ASSERT(condition, error) if (!(condition)) { \
        VOXEL_ERROR_MESSAGE("Voxel error: Assertion failed - ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (error); \
    }

#define VOXEL_THROW(error) do { \
        VOXEL_ERROR_MESSAGE("Voxel error: ", voxel_lookupError(error), __func__, __FILE__, __LINE__); \
        return (error); \
    } while (0)

#define VOXEL_MUST(result) do { \
        int resultValue = (result); \
        if (resultValue) { \
            VOXEL_ERROR_MESSAGE("   ", "", __func__, __FILE__, __LINE__); \
            return resultValue; \
        } \
    } while (0)

#define VOXEL_OK 0
#define VOXEL_ERROR_NO_CODE -1
#define VOXEL_ERROR_TOKENISATION_BYTE -2
#define VOXEL_ERROR_TOKENISATION_END -3
#define VOXEL_ERROR_TYPE_MISMATCH -4
#define VOXEL_ERROR_NOT_IMPLEMENTED -5

const char* voxel_lookupError(VOXEL_ERROR_CODE error) {
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

        default:
            return "Unknown error";
    }
}