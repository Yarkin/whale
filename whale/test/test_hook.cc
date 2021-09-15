#include <whale.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>

char *(*Origin_getenv)(const char *);
void *(*Origin_malloc)(size_t size);
void (*Origin_free)(void *);
char *Hooked_getenv(const char *name) {
    if (!strcmp(name, "lody")) {
        return strdup("are you ok?");
    }
    char *(*O)(const char *) = Origin_getenv;
    return O(name);
}

void *Hooked_malloc(size_t size) {
    void *(*Omalloc)(size_t size)= Origin_malloc;
    void * ptr = Omalloc(size);
    printf("malloc:%p size:%zu\n", ptr, size);
    return ptr;
}

void Hooked_free(void* ptr) {
    void (*Ofree)(void *) = Origin_free;
    Ofree(ptr);
    printf("free:%p\n", ptr);
    return;
}

int main() {
#if defined(__APPLE__)
    void *handle = dlopen("libc.dylib", RTLD_NOW);
#else
    void *handle = dlopen("libc.so", RTLD_NOW);
#endif
    assert(handle != nullptr);
    void *symbol = dlsym(handle, "getenv");
    assert(symbol != nullptr);
    WInlineHookFunction(
            symbol,
            reinterpret_cast<void *>(Hooked_getenv),
            reinterpret_cast<void **>(&Origin_getenv)
    );
    printf("hook getenv\n");
    void *m_symbol = dlsym(handle, "__libc_malloc");
    printf("malloc symbol:%p\n", m_symbol);
    assert(m_symbol != nullptr);
    WInlineHookFunction(
            m_symbol,
            reinterpret_cast<void *>(Hooked_malloc),
            reinterpret_cast<void **>(&Origin_malloc)
    );
    printf("hook malloc\n");
    void *f_symbol = dlsym(handle, "free");
    assert(f_symbol != nullptr);
    WInlineHookFunction(
            f_symbol,
            reinterpret_cast<void *>(Hooked_free),
            reinterpret_cast<void **>(&Origin_free)
    );
    printf("hook free\n");
//    WImportHookFunction(
//            "_getenv",
//            reinterpret_cast<void *>(Hooked_getenv),
//            reinterpret_cast<void **>(&Origin_getenv)
//    );

    const char *val = getenv("lody");
    std::cout << val;

    void * ptr = malloc(20);
    free(ptr);
    return 0;
}
