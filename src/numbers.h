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