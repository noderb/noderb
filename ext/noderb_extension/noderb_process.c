#include <noderb_common.h>
#include <noderb_process.h>
#include <noderb.h>
#include <noderb_tools.h>

typedef struct {
    NODERB_BASIC_HANDLE
    uv_pipe_t* stdout;
    uv_pipe_t* stdin;
    uv_pipe_t* stderr;
} nodeRb_process_handle;

typedef struct {
    NODERB_BASIC_HANDLE
} nodeRb_process_read_handle;

void nodeRb_process_close(uv_handle_t* handle) {
    nodeRb_process_handle* process_handle = (nodeRb_process_handle*) handle->data;
    free(process_handle->stdin);
    free(process_handle->stdout);
    free(process_handle->stderr);
    free(process_handle);
    free(handle);
}

VALUE nodeRb_process_write(VALUE self, VALUE data) {
    // Read data from Ruby
    uv_process_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_process_t, handle);
    uv_write_t write_req;
    uv_buf_t buf;
    // Get data to write from Ruby
    char* buffer = rb_string_value_cstr(&data);
    // Get process handle
    nodeRb_process_handle* process_handle = (nodeRb_process_handle*) handle->data;
    // Write data to stream
    nodeRb_write((uv_stream_t*) process_handle->stdin, buffer, sizeof(buffer));
}

VALUE nodeRb_process_kill(VALUE self, VALUE signal) {
    uv_process_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_process_t, handle);
    uv_process_kill(handle, (int) rb_num2int(signal));
}

void nodeRb_process_exit(uv_process_t* handle, int status, int signal) {
    nodeRb_process_handle* process_handle = (nodeRb_process_handle*) handle->data;
    VALUE clazz = nodeRb_get_class_from_id(process_handle->target);
    rb_funcall(clazz, rb_intern("on_exit"), 2, rb_int2inum(status), rb_int2inum(signal));
}

VALUE nodeRb_startProcess(VALUE self, VALUE executable, VALUE arguments, VALUE environment, VALUE directory, VALUE clazz) {
    uv_process_options_t options = {0};
    // On process exit
    options.exit_cb = nodeRb_process_exit;
    // Executable
    options.file = rb_string_value_cstr(&executable);
    // Arguments
    long len;
    len = RARRAY_LEN(arguments);
    char* args[len + 2];
    args[0] = rb_string_value_cstr(&executable);
    args[len + 1] = NULL;
    options.args = args;
    if (len > 0) {
        long i;
        VALUE *arr = RARRAY_PTR(arguments);
        for (i = 0; i < len; i++) {
            options.args[i + 1] = rb_string_value_cstr(&arr[i]);
        }
    }
    // Environment
    len = RARRAY_LEN(environment);
    char* env[len+1];
    options.env = env;
    options.env[len] = NULL;
    if (len > 0) {
        long i;
        VALUE *arr = RARRAY_PTR(environment);
        for (i = 0; i < len; i++) {
            options.env[i] = rb_string_value_cstr(&arr[i]);
        }
    }
    // Working directory
    options.cwd = rb_string_value_cstr(&directory);
    // NodeRb helpers
    nodeRb_process_handle* process_handle = malloc(sizeof(nodeRb_process_handle));
    process_handle->target = rb_num2long(rb_obj_id(clazz));
    // Be safe of GC
    nodeRb_register_instance(clazz);
    // stdin
    uv_pipe_t* stdin = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(stdin);
    options.stdin_stream = stdin;
    process_handle->stdin = stdin;
    // stdout
    uv_pipe_t* stdout = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(stdout);
    options.stdout_stream = stdout;
    nodeRb_process_read_handle* stdout_handle = malloc(sizeof(nodeRb_process_read_handle));
    stdout_handle->target = process_handle->target;
    stdout_handle->target_callback = (char*) "on_stdout";
    stdout->data = stdout_handle;
    process_handle->stdout = stdout;
    // stderr
    uv_pipe_t* stderr = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(stderr);
    options.stderr_stream = stderr;
    nodeRb_process_read_handle* stderr_handle = malloc(sizeof(nodeRb_process_read_handle));
    stderr_handle->target = process_handle->target;
    stderr_handle->target_callback = (char*) "on_stderr";
    stderr->data = stderr_handle;
    process_handle->stderr = stderr;
    // libuv handle
    uv_process_t* handle = malloc(sizeof(uv_process_t));
    handle->data = process_handle;
    // Save reference into Ruby object
    rb_iv_set(clazz, "@_handle", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, handle));
    // spawn process
    uv_spawn(handle, options);
    // Listen to stdout/err
    uv_read_start((uv_stream_t*) stdout, nodeRb_read_alloc, nodeRb_read);
    uv_read_start((uv_stream_t*) stderr, nodeRb_read_alloc, nodeRb_read);
    // call back ruby
    rb_funcall(clazz, rb_intern("on_start"), 0);
};
