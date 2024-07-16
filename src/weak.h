voxel_Thing* voxel_newWeakRef(voxel_Context* context, voxel_Thing* target) {
    voxel_Thing* thing = voxel_newThing(context); VOXEL_TAG_NEW_THING(VOXEL_TYPE_WEAK);

    thing->type = VOXEL_TYPE_WEAK;
    thing->value = (void*)target;

    voxel_pushOntoList(context, context->weakRefs, thing);

    voxel_unreferenceThing(context, thing);

    return thing;
}

VOXEL_ERRORABLE voxel_destroyWeakRef(voxel_Context* context, voxel_Thing* thing) {
    voxel_List* weakRefsList = (voxel_List*)context->weakRefs->value;
    voxel_ListItem* currentListItem = weakRefsList->firstItem;

    while (currentListItem) {
        if (currentListItem->value == thing) {
            if (currentListItem == weakRefsList->firstItem) {
                weakRefsList->firstItem = currentListItem->nextItem;
            }

            if (currentListItem == weakRefsList->lastItem) {
                weakRefsList->lastItem = currentListItem->previousItem;
            }

            if (currentListItem->previousItem) {
                currentListItem->previousItem->nextItem = currentListItem->nextItem;
            }

            if (currentListItem->nextItem) {
                currentListItem->nextItem->previousItem = currentListItem->previousItem;
            }

            VOXEL_FREE(currentListItem); VOXEL_TAG_FREE(voxel_ListItem);

            break;
        }

        currentListItem = currentListItem->nextItem;
    }

    VOXEL_TAG_DESTROY_THING(VOXEL_TYPE_WEAK);

    VOXEL_FREE(thing); VOXEL_TAG_FREE(voxel_Thing);

    return VOXEL_OK;
}

voxel_Thing* voxel_dereferenceWeakRef(voxel_Context* context, voxel_Thing* thing) {
    if (thing->type != VOXEL_TYPE_WEAK) {
        return thing;
    }

    voxel_Thing* dereferencedThing = (voxel_Thing*)thing->value;

    if (dereferencedThing) {
        dereferencedThing->referenceCount++;

        return dereferencedThing;
    }

    return voxel_newNull(context);
}

voxel_Bool voxel_compareWeakRefs(voxel_Thing* a, voxel_Thing* b) {
    return a->value == b->value;
}

voxel_Thing* voxel_copyWeakRef(voxel_Context* context, voxel_Thing* thing) {
    return voxel_newWeakRef(context, (voxel_Thing*)thing->value);
}

VOXEL_ERRORABLE voxel_weakRefToString(voxel_Context* context, voxel_Thing* thing) {
    return VOXEL_OK_RET(voxel_newStringTerminated(context, (voxel_Byte*)"(weak)"));
}

voxel_Bool voxel_weakRefIsTruthy(voxel_Thing* thing) {
    return thing->value != VOXEL_NULL;
}

void voxel_unreferenceFromWeakRefs(voxel_Context* context, voxel_Thing* thing) {
    voxel_List* weakRefsList = (voxel_List*)context->weakRefs->value;
    voxel_ListItem* currentListItem = weakRefsList->firstItem;

    while (currentListItem) {
        voxel_Thing* weakRef = currentListItem->value;

        if ((voxel_Thing*)weakRef->value == thing) {
            weakRef->value = VOXEL_NULL;
        }

        currentListItem = currentListItem->nextItem;
    }
}