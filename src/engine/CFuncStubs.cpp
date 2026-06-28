// C runtime function stubs for Symbian GCCE
// These must be actual function definitions, not inline, to satisfy linker references
// from code that includes <stdlib.h> (which declares extern malloc/free)

#include <e32def.h>
#include <stdlib.h>

extern "C" {
    void* malloc(size_t s) { (void)s; return NULL; }
    void free(void* p) { (void)p; }
}
