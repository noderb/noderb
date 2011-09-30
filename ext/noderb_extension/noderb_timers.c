#include <noderb_common.h>
#include <noderb_timers.h>

#include <noderb_tools.h>
#include <noderb.h>

typedef struct {
    long target;
    int repeat;
} nodeRb_timers_handle;

void nodeRb_timers_callback(uv_timer_t* handle, int status){
    // Load data
    nodeRb_timers_handle* data = (nodeRb_timers_handle*) handle->data;
    VALUE handler = nodeRb_get_class_from_id(data->target);
    // Run callback
    rb_funcall(nodeRb_get_nodeRb_module(), rb_intern("next_tick_schedule"), 2, handler, handler);
    if(data->repeat == 0){
        // Let the GC work
        nodeRb_unregister_instance(handler);
        // Free memory
        free(data);
        free(handle);
    }
}

VALUE nodeRb_timers_stop(VALUE self){
    // Load data
    uv_timer_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_timer_t, handle);
    // Free memory
    nodeRb_timers_handle* data = (nodeRb_timers_handle*) handle->data;
    free(data);
    // Stop timer
    uv_timer_stop(handle);
    // Let the GC work
    nodeRb_unregister_instance(self);
}

VALUE nodeRb_timers_once(VALUE self, VALUE timeout, VALUE repeat, VALUE handler){
    // Allocate memory
    uv_timer_t* handle = (uv_timer_t*) malloc(sizeof(uv_timer_t));
    nodeRb_timers_handle* data = (nodeRb_timers_handle*) malloc(sizeof(nodeRb_timers_handle));
    // Save from GC
    nodeRb_register_instance(handler);
    // Save data for later use
    if(rb_num2long(timeout) > 0){
        data->repeat = 1;
    }
    data->target = rb_num2long(rb_obj_id(handler));
    // Initialize
    uv_timer_init(uv_default_loop(), handle);
    handle->data = data;
    // Save handle
    rb_iv_set(handler, "@_handle", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, handle));
    // Schedule
    uv_timer_start(handle, nodeRb_timers_callback, rb_num2long(timeout), rb_num2long(repeat));
}