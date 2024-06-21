VOXEL_ERRORABLE voxel_registerEnumEntry(voxel_Context* context, voxel_Thing* value, voxel_Thing* identifier) {
    return voxel_setObjectItem(context, context->enumLookup, value, identifier);
}

voxel_Thing* voxel_getEnumEntryFromLookup(voxel_Context* context, voxel_Thing* value) {
    voxel_ObjectItem* enumLookupObjectItem = voxel_getObjectItem(context->enumLookup, value);

    return enumLookupObjectItem ? enumLookupObjectItem->value : value;
}