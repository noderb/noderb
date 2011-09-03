#include <noderb_common.h>

#include <noderb_defer.h>
#include <noderb_tools.h>

typedef struct {
    long worker;
    long callback;
    long result;
} nodeRb_defer_handle;

void nodeRb_defer_work_done(uv_work_t* req){
//    // Load data
//    nodeRb_defer_handle* data = (nodeRb_defer_handle*) req->data;
//    printf("Finish: %p\n", data);
//    VALUE result = nodeRb_get_class_from_id(data->result);
//    VALUE callback = nodeRb_get_class_from_id(data->callback);
//    // Call the callback
//    rb_funcall(callback, rb_intern("call"), 1, result);
//    // Clean up
//    nodeRb_unregister_instance(callback);
//    nodeRb_unregister_instance(result);
//    free(data);
//    free(req);
}

void nodeRb_defer_work(uv_work_t* req){
//    // Load data
//    nodeRb_defer_handle* data = (nodeRb_defer_handle*) req->data;
//    VALUE callback = nodeRb_get_class_from_id(data->worker);
//    printf("Run: %p\n", data);
//    // Call the callback
//    rb_funcall(callback, rb_intern("start"), 0, 0);
//    // Clean up
//    printf("Run2: %p\n", data);
//    nodeRb_unregister_instance(callback);
//    data->worker = 0;
}

VALUE nodeRb_defer(VALUE self, VALUE worker, VALUE callback){
//    // Initialize memory
//    uv_work_t* handle = (uv_work_t*) malloc(sizeof(uv_work_t));
//    nodeRb_defer_handle* data = (nodeRb_defer_handle*) malloc(sizeof(nodeRb_defer_handle));
//    printf("Request: %p\n", data);
//    // Safe from GC
//    nodeRb_register_instance(worker);
//    nodeRb_register_instance(callback);
//    // Save data
//    data->worker = rb_num2long(rb_obj_id(worker));
//    data->callback = rb_num2long(rb_obj_id(callback));
//    handle->data = data;
//    // Queue the work
//    uv_queue_work(handle, nodeRb_defer_work, nodeRb_defer_work_done);
}