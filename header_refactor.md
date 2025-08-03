# Header Refactoring Strategy for OCTypes

## Current Problem
The OCTypes library has circular dependency issues between headers, making it difficult to:
- Build reliably across different environments
- Maintain clear dependency relationships
- Ensure fast compilation times
- Add new modules without breaking existing code

## Proposed Solution: Layered Architecture

### Philosophy: "Foundation Up, Dependencies Down"

Create a clear hierarchy where dependencies only flow upward, never creating cycles.

## Layer 1: Foundation Header (`OCType.h`)

**Purpose**: Provide the minimal foundation that everything else builds on.

**Contains:**
```c
// Basic primitive types
typedef uint16_t OCTypeID;
typedef size_t OCIndex;

// All forward declarations for opaque types
typedef const struct impl_OCString *OCStringRef;
typedef const struct impl_OCMutableString *OCMutableStringRef;
typedef const struct impl_OCArray *OCArrayRef;
typedef const struct impl_OCMutableArray *OCMutableArrayRef;
// ... etc for all OCTypes

// Essential shared structures
typedef struct {
    uint64_t location;
    uint64_t length;
} OCRange;

// Core enums and function pointers
typedef enum {
    kOCCompareLessThan = -1,
    kOCCompareEqualTo = 0,
    kOCCompareGreaterThan = 1,
    // ...
} OCComparisonResult;

typedef OCComparisonResult (*OCComparatorFunction)(const void *val1, const void *val2, void *context);

// Memory management primitives
const void *OCRetain(const void *ptr);
void OCRelease(const void *ptr);
// ... other core functions
```

**Rules:**
- ✅ NO implementation details
- ✅ NO circular dependencies
- ✅ Only includes system headers and cJSON.h
- ✅ Must compile independently

## Layer 2: Module Headers (`OCString.h`, `OCArray.h`, etc.)

**Purpose**: Define the public API for each specific module.

**Template:**
```c
#ifndef OCSTRING_H
#define OCSTRING_H
#include "OCType.h"     // Foundation types and forward declarations
#include "cJSON.h"      // If needed for this module's API

#ifdef __cplusplus
extern "C" {
#endif

// Module-specific API declarations
OCStringRef OCStringCreateWithCString(const char *cStr);
// ... other functions

#ifdef __cplusplus
}
#endif
#endif
```

**Rules:**
- ✅ Include `OCType.h` for foundation types
- ✅ Include only direct dependencies (e.g., `cJSON.h` if API uses it)
- ✅ Use forward declarations from `OCType.h` for other OCTypes
- ❌ **NEVER include `OCTypes.h`** (circular dependency trap)
- ✅ Keep APIs minimal and focused
- ✅ Must compile independently

## Layer 3: Implementation Files (`.c`)

**Purpose**: Contain the actual implementation logic.

**Template:**
```c
#include "OCString.h"       // Own header FIRST (ensures self-containment)
#include <stdio.h>          // System headers
#include <stdlib.h>
#include <string.h>
#include "OCArray.h"        // Other project headers as needed
#include "OCDictionary.h"
// Can include OCTypes.h for convenience if many headers needed

// Full struct definitions (private to implementation)
struct impl_OCString {
    OCBase base;
    char *data;
    size_t length;
    // ...
};

// Implementation code...
```

**Rules:**
- ✅ **Always include own header FIRST** (ensures header self-containment)
- ✅ Include system headers next
- ✅ Include all project headers actually used
- ✅ Can include `OCTypes.h` for convenience if needed
- ✅ Access full struct definitions (either local or via private headers)

## Layer 4: Convenience Header (`OCTypes.h`)

**Purpose**: "Kitchen sink" header for end users and complex implementations.

**Template:**
```c
#ifndef OCTypes_h
#define OCTypes_h

// Foundation first
#include "OCType.h"

// All module headers in dependency order
#include "cJSON.h"
#include "OCString.h"
#include "OCArray.h"
#include "OCDictionary.h"
#include "OCNumber.h"
#include "OCIndexArray.h"
// ... etc

// Additional convenience types/macros if needed

#endif
```

**Rules:**
- ✅ Include `OCType.h` first
- ✅ Include all module headers in dependency order
- ✅ Provide additional convenience types/macros
- ✅ **Only used by .c files and end-user code**
- ❌ **Never included by other headers**

## Dependency Flow

```
┌─────────────────┐
│   OCType.h      │  ←── Foundation (system headers, cJSON.h)
│   (Foundation)  │
└─────────────────┘
          ↑
┌─────────────────┐
│ OCString.h      │  ←── Individual modules (OCType.h, specific deps)
│ OCArray.h       │
│ OCNumber.h      │
│ etc.            │
└─────────────────┘
          ↑
┌─────────────────┐
│ *.c files       │  ←── Implementations (own header + all needed)
│ (Implementation)│
└─────────────────┘
          ↑
┌─────────────────┐
│ OCTypes.h       │  ←── Convenience (everything)
│ (Kitchen Sink)  │
└─────────────────┘
```

## Migration Strategy

### Phase 1: Foundation Setup ✅
- [x] Move core types to `OCType.h`
- [x] Add all forward declarations
- [x] Move shared structures (`OCRange`, `OCComparatorFunction`)
- [x] Remove duplicates from `OCTypes.h`

### Phase 2: Fix Individual Headers ✅
- [x] Update each module header to only include `OCType.h` + direct deps
- [x] Remove `OCTypes.h` includes from headers
- [x] Add forward declarations where needed
- [x] Test each header compiles independently
- [x] Fix `OCNumberType` redefinition issue in `OCIndexArray.h`
- [x] Correct mutable type forward declarations in `OCType.h`

### Phase 3: Fix Implementation Files ✅
- [x] Ensure each `.c` file includes own header first
- [x] Add explicit includes for all dependencies
- [x] Remove reliance on transitive includes
- [x] Add missing function declarations (`cleanupConstantStringTable`)

### Phase 4: Cleanup and Verification ✅
- [x] Update `OCTypes.h` to proper convenience header
- [x] Build and test entire project
- [x] Update build system if needed
- [x] Document new include patterns
- [x] All tests pass successfully

## Benefits

1. **Compilation Speed**: Headers stay minimal, reducing parsing time
2. **Maintainability**: Clear dependency chains, easy to modify
3. **Modularity**: Each module can be understood independently
4. **Flexibility**: Users can include just what they need
5. **Error Prevention**: Circular dependencies become impossible
6. **Build Reliability**: Consistent compilation across environments

## Potential Issues and Solutions

### Issue: Some headers need types from multiple modules
**Solution**: Use forward declarations in headers, full includes in .c files

### Issue: Implementation needs access to struct internals
**Solution**: Define full structs in .c files or create private headers

### Issue: Circular type dependencies
**Solution**: Move shared types to foundation layer (`OCType.h`)

### Issue: Complex build dependencies
**Solution**: Clear layered approach makes dependencies explicit and manageable

## Testing Strategy

1. **Header Independence**: Each header must compile with just its declared dependencies
2. **Implementation Completeness**: Each .c file must include everything it needs
3. **No Transitive Dependencies**: No relying on headers pulled in by other headers
4. **Build Verification**: Full clean build must succeed
5. **Functionality Tests**: All existing tests must continue to pass

## Success Criteria

- ✅ All headers compile independently
- ✅ No circular dependencies
- ✅ Clean separation of concerns
- ✅ Fast compilation times
- ✅ All tests pass
- ✅ Consistent patterns across modules
- ✅ Easy to add new modules

## Example: Correct Usage Patterns

### End-user code:
```c
#include "OCTypes.h"  // Gets everything

int main() {
    OCStringRef str = OCStringCreateWithCString("hello");
    // ...
}
```

### Module implementation:
```c
#include "OCString.h"   // Own header first
#include <string.h>     // System headers
#include "OCArray.h"    // Other modules as needed

// Implementation...
```

### New module header:
```c
#include "OCType.h"     // Foundation only
// API declarations using forward-declared types
```

This approach follows the **CoreFoundation pattern** - a proven architecture for large-scale C libraries that need to be both powerful and maintainable.
