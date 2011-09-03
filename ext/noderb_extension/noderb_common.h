#include <libuv/include/uv.h>
#include <ruby.h>

#define NODERB_BASIC_HANDLE \
    long target; \
    char* target_callback; \
    int proxy_enabled; \
    void* proxy_target;

typedef struct {
    NODERB_BASIC_HANDLE
} nodeRb_basic_handle;

#define rb_num2int(x) NUM2LONG(x)