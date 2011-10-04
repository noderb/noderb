#include <noderb_common.h>

#include <noderb_dns.h>
#include <noderb_tools.h>

typedef struct {
    VALUE target;
} nodeRb_dns_handle;

void nodeRb_dns_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res){
    // Load data
    nodeRb_dns_handle* data = (nodeRb_dns_handle*) handle->data;
    // Get resolved address
    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(((struct sockaddr_in*) res->ai_addr)->sin_addr), address, INET_ADDRSTRLEN);
    // Call back
    rb_funcall(data->target, rb_intern("call"), 1, rb_str_new2(address));
    // Let callback GC
    nodeRb_unregister_instance(data->target);
    free(handle);
    free(data);
};

VALUE nodeRb_dns_resolve(VALUE self, VALUE host, VALUE callback){
    // Allocate memory
    uv_getaddrinfo_t* handle = malloc(sizeof(uv_getaddrinfo_t));
    nodeRb_dns_handle* data = malloc(sizeof(nodeRb_dns_handle));
    // Save from GC
    nodeRb_register_instance(callback);
    // Save data
    data->target = callback;
    // Save callback
    handle->data = data;
    // Resolve
    uv_getaddrinfo(uv_default_loop(), handle, nodeRb_dns_resolved, rb_string_value_cstr(&host), NULL, NULL);
    return self;
}