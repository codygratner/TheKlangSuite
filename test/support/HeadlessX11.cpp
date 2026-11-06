#include "HeadlessX11.h"

#if defined (__linux__)
#include <atomic>
#include <cstdio>
#include <dlfcn.h>

namespace {
    std::atomic<bool> installed { false };
    std::atomic<std::uint32_t> errorCount { 0 };
    std::atomic<unsigned char> lastError { 0 }, lastRequest { 0 };

    // Real X11 protocol function pointer types defined locally with zero macro pollution:
    extern "C" {
        typedef int (*XErrorHandlerFn)(void* display, void* errorEvent);
        typedef XErrorHandlerFn (*XSetErrorHandlerFn)(XErrorHandlerFn);
        typedef int (*XInitThreadsFn)(void);
    }

    struct MinimalXErrorEvent {
        int type;
        void* display;
        unsigned long resourceid;
        unsigned long serial;
        unsigned char error_code;
        unsigned char request_code;
        unsigned char minor_code;
    };

    int onXError(void* /*display*/, void* rawEvent) {
        errorCount.fetch_add(1, std::memory_order_relaxed);
        if (rawEvent != nullptr) {
            auto* e = static_cast<MinimalXErrorEvent*>(rawEvent);
            lastError.store(e->error_code, std::memory_order_relaxed);
            lastRequest.store(e->request_code, std::memory_order_relaxed);
        }
        return 0; // Never exit(1); swallows non-fatal X errors headlessly
    }
}

void HeadlessX11::install() noexcept {
    if (installed.exchange(true))
        return;

    void* lib = dlopen("libX11.so.6", RTLD_NOW | RTLD_LOCAL);
    if (lib == nullptr)
        lib = dlopen("libX11.so", RTLD_NOW | RTLD_LOCAL);

    if (lib == nullptr) {
        std::fprintf(stderr, "[HeadlessX11] dlopen libX11 failed: %s\n", dlerror());
        return;
    }

    auto initThreads = reinterpret_cast<XInitThreadsFn>(dlsym(lib, "XInitThreads"));
    auto setHandler  = reinterpret_cast<XSetErrorHandlerFn>(dlsym(lib, "XSetErrorHandler"));

    if (initThreads != nullptr) initThreads(); // Mandatory before first XOpenDisplay
    if (setHandler != nullptr)  setHandler(onXError);
}

bool HeadlessX11::isInstalled() noexcept { return installed.load(); }
std::uint32_t HeadlessX11::swallowedErrorCount() noexcept { return errorCount.load(); }
unsigned char HeadlessX11::lastErrorCode() noexcept       { return lastError.load(); }
unsigned char HeadlessX11::lastRequestCode() noexcept     { return lastRequest.load(); }

#else
void HeadlessX11::install() noexcept {}
bool HeadlessX11::isInstalled() noexcept { return false; }
std::uint32_t HeadlessX11::swallowedErrorCount() noexcept { return 0; }
unsigned char HeadlessX11::lastErrorCode() noexcept       { return 0; }
unsigned char HeadlessX11::lastRequestCode() noexcept     { return 0; }
#endif
