#include <noderb_common.h>
#include <noderb_tools.h>
#include <noderb.h>

/*
    Consolidate read and write request to allow cross-functionality like proxy
*/

// Allocates memory exactly as libuv suggests, this might be one area of optimization
uv_buf_t nodeRb_read_alloc(uv_handle_t* handle, size_t suggested_size) {
    uv_buf_t buf;
    buf.base = (char*) malloc(suggested_size);
    buf.len = suggested_size;
    return buf;
}

void nodeRb_on_close(uv_handle_t* client) {
    free(client->data);
    free(client);
}

// Called when data read from stream
void nodeRb_read(uv_stream_t* uv_handle, ssize_t nread, uv_buf_t buf) {
    nodeRb_basic_handle* handle = (nodeRb_basic_handle*) uv_handle->data;
    if (nread < 0) {
        if (buf.base) {
            free(buf.base);
            //uv_close((uv_handle_t*) uv_handle, nodeRb_on_close);
        }
        return;
    }
    if (nread == 0) {
        free(buf.base);
        return;
    }
    if(handle->proxy_enabled == 1){
        nodeRb_write(uv_handle, buf.base, nread);
    }else{
        rb_funcall(nodeRb_get_class_from_id(handle->target), rb_intern(handle->target_callback), 1, rb_str_new(buf.base, nread));
        free(buf.base);
    }

}

// Requests write to stream
void nodeRb_write(uv_stream_t* handle, char* data, size_t len){
    if(handle->type != UV_TCP && handle->type != UV_NAMED_PIPE){
        return;
    }
    write_req_t* wr = malloc(sizeof(write_req_t));
    wr->buf.base = data;
    wr->buf.len = len;
    uv_write(&wr->req, handle, &wr->buf, 1, nodeRb_after_write);
}

// Called after data written to stream
void nodeRb_after_write(uv_write_t* req, int status) {
    if (status) {
        uv_err_t err = uv_last_error(uv_default_loop());
        fprintf(stderr, "uv_write error: %s\n", uv_strerror(err));
    }
    write_req_t* wr = (write_req_t*) req;
    free(wr);
}

VALUE nodeRb_startProxy(VALUE self, VALUE source, VALUE target){
    // Read data saved in Ruby
    uv_stream_t *source_handle;
    Data_Get_Struct(rb_iv_get(source, "@_proxy_target"), uv_stream_t, source_handle);
    uv_stream_t *target_handle;
    Data_Get_Struct(rb_iv_get(target, "@_proxy_target"), uv_stream_t, target_handle);
    // Get internal structures
    nodeRb_basic_handle* data = (nodeRb_basic_handle*) source_handle->data;
    // Enable proxy
    data->proxy_enabled = 1;
    data->proxy_target = target_handle;
}

VALUE nodeRb_stopProxy(VALUE self, VALUE source){
    // Read data saved in Ruby
    uv_stream_t *source_handle;
    Data_Get_Struct(rb_iv_get(source, "@_proxy_target"), uv_stream_t, source_handle);
    // Get internal structures
    nodeRb_basic_handle* data = (nodeRb_basic_handle*) source_handle->data;
    // Disable proxy
    data->proxy_enabled = 0;
}

/*
    NodeRb.next_tick runs block of code on the next iteration of the loop
*/

// Indicator that we already have something to run
int nodeRbNextTickStatus = 0;

// Invoked by the loop to execute scheduled blocks in Ruby
void nodeRb_next_tick(uv_idle_t* handle, int status) {
    uv_idle_stop(handle);
    free(handle);
    nodeRbNextTickStatus = 0;
    rb_funcall(nodeRb_get_nodeRb_module(), rb_intern("next_tick_execute"), 0);
}

// Schedules invocation on the next iteration if not already scheduled
VALUE nodeRb_nextTick(VALUE self) {
    if(nodeRbNextTickStatus == 0){
        uv_idle_t* handle = malloc(sizeof(uv_idle_t));
        uv_idle_init(uv_default_loop(), handle);
        nodeRbNextTickStatus = 1;
        uv_idle_start(handle, nodeRb_next_tick);
    }
}

/*
    Tools to simplify NodeRb development
*/

// Register object to be GC safe
VALUE nodeRb_register_instance(VALUE instance) {
    rb_funcall(nodeRb_get_nodeRb_module(), rb_intern("register_instance"), 1, instance);
    //rb_ary_push(rb_iv_get(nodeRb_get_nodeRb_module(), "@instances"), instance);
}

// Ruby wrapper for previous function
VALUE nodeRb_registerInstance(VALUE self, VALUE instance) {
    nodeRb_register_instance(instance);
}

// Unregister object to be GC safe
VALUE nodeRb_unregister_instance(VALUE instance) {
    rb_funcall(nodeRb_get_nodeRb_module(), rb_intern("unregister_instance"), 1, instance);
    //rb_ary_delete(rb_iv_get(nodeRb_get_nodeRb_module(), "@instances"), instance);
}

// Ruby wrapper for previous function
VALUE nodeRb_unregisterInstance(VALUE self, VALUE instance) {
    nodeRb_unregister_instance(instance);
}

// Transforms Ruby Object.object_id back into the Object
VALUE nodeRb_get_class_from_id(long id) {
    return rb_funcall(rb_const_get(rb_cObject, rb_intern("ObjectSpace")), rb_intern("_id2ref"), 1, rb_int2inum(id));
}

// Used for handling errors inside NodeRb
int nodeRb_handle_error(int e) {
    if (e) {
        uv_err_t error = uv_last_error(uv_default_loop());
        fprintf(stderr, "%s\n", uv_strerror(error));
        return 1;
    } else {
        return 0;
    }
};

// Starts the event loop and adds one reference to ensure loop will run
VALUE nodeRb_start(VALUE self) {
    uv_ref(uv_default_loop());
    if (rb_block_given_p()) {
        rb_yield(Qnil);
    }
    uv_run(uv_default_loop());
};
