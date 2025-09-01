# OCTypes JSON Typed Serialization Refactor Plan

## Objective
Implement comprehensive typed JSON serialization across all OCType classes to enable full round-trip fidelity while maintaining backward compatibility.

## Current State
- ✅ `OCDataCopyAsJSONTyped`/`OCDataCreateFromJSONTyped` implemented
- ❌ All other OCType classes use untyped JSON (information loss)

## Target Architecture

### Core Pattern
```c
// For each OCType class:
cJSON* OCXxxCopyJSONTyped(OCXxxRef obj);
OCXxxRef OCXxxCreateFromJSONTyped(cJSON* json);

// Typed format:
{"type": "OCXxx", "value": {...}, "metadata": {...}}
```

### Implementation Priority
1. **OCNumber** - Handle complex numbers, preserve type info
2. **OCBoolean** - Distinguish from generic true/false  
3. **OCString** - Add encoding metadata
4. **OCArray** - Recursive typed serialization for elements
5. **OCDictionary** - Typed keys and values
6. **OCSet** - Preserve set semantics
7. **OCIndexSet/OCIndexArray** - Specialized collections

### Backward Compatibility
- Keep existing `OCXxxCopyJSON`/`OCXxxCreateFromJSON` functions
- Typed functions handle both formats on deserialization
- Legacy format continues to work seamlessly

### Testing Strategy
- Round-trip tests for all data types
- Mixed format compatibility tests
- Performance benchmarks vs legacy functions

## Benefits
- ✅ Zero information loss in serialization
- ✅ Type safety and validation
- ✅ Future-proof architecture
- ✅ Backward compatibility maintained
