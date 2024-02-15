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

    if (length > 0) {
        voxel_copy(data, string->value, length);
    }

    return thing;
}