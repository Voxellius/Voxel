voxel_Scope* voxel_newScope(voxel_Context* context, voxel_Scope* parentScope) {
    voxel_Scope* scope = (voxel_Scope*)VOXEL_MALLOC(sizeof(voxel_Scope)); VOXEL_TAG_MALLOC(voxel_Scope);

    scope->context = context;
    scope->parentScope = parentScope;
    scope->things = voxel_newObject(context);

    return scope;
}

VOXEL_ERRORABLE voxel_destroyScope(voxel_Scope* scope) {
    VOXEL_MUST(voxel_unreferenceThing(scope->context, scope->things));

    VOXEL_FREE(scope); VOXEL_TAG_FREE(voxel_Scope);

    return VOXEL_OK;
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