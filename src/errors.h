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
#define VOXEL_ERROR_TOKENISATION -2

const char* voxel_lookupError(VOXEL_ERROR_CODE error) {
    switch (error) {
        case VOXEL_ERROR_NO_CODE:
            return "No code loaded";

        case VOXEL_ERROR_TOKENISATION:
            return "Error encountered when tokenising";

        default:
            return "Unknown error";
    }
}