# JSON CreateFromJSON Function Signature Updates

## Overview
Updating all `CreateFromJSON` functions to include `OCStringRef *outError` parameter for proper error handling.

## Changes Required

### OCTypes Project
- [x] `OCRegisterType` - already has correct signature with `(cJSON *, OCStringRef *)`
- [x] `OCTypeCreateFromJSONTyped` - needs to be updated to accept `OCStringRef *outError`
- [ ] `OCStringCreateFromJSON` - needs `OCStringRef *outError` parameter
- [ ] `OCBooleanCreateFromJSON` - needs `OCStringRef *outError` parameter  
- [ ] `OCNumberCreateFromJSON` - needs `OCStringRef *outError` parameter
- [ ] `OCArrayCreateFromJSONTyped` - needs `OCStringRef *outError` parameter
- [ ] `OCDictionaryCreateFromJSONTyped` - needs `OCStringRef *outError` parameter

### SITypes Project
- [ ] All `CreateFromJSON` functions need `OCStringRef *outError` parameter

### RMNLib Project
- [x] `GeographicCoordinateCreateFromJSON` - already has correct signature
- [x] `SparseSamplingCreateFromJSON` - already has correct signature
- [x] `DatasetCreateFromJSON` - already has correct signature
- [ ] Other types need to be checked and updated

## Function Signature Pattern
```c
// Old signature (remove these)
TypeRef TypeCreateFromJSON(cJSON *json);

// New signature (update to this)
TypeRef TypeCreateFromJSON(cJSON *json, OCStringRef *outError);
```

## Notes
- No backward compatibility needed (pre-alpha)
- All factory functions registered with `OCRegisterType` must have new signature
- Error handling should be consistent across all types
