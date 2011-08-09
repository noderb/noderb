#include <noderb.h>

VALUE nodeRb;
VALUE nodeRbPointer;
VALUE nodeRbConnection;
VALUE nodeRbProcess;

VALUE nodeRbModules;

int nodeRbNextTickStatus = 0;

typedef struct {
    
} nodeRb_next_tick_handle;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

typedef struct {
    long connection;
} nodeRb_client;

typedef struct {
    long target;
    uv_pipe_t* stdout;
    uv_pipe_t* stdin;
    uv_pipe_t* stderr;
} nodeRb_process_handle;


VALUE nodeRb_register_instance(VALUE self, VALUE instance) {
    rb_ary_push(rb_iv_get(self, "@instances"), instance);
}

VALUE nodeRb_unregister_instance(VALUE self, VALUE instance) {
    rb_ary_delete(rb_iv_get(self, "@instances"), instance);
}

VALUE nodeRb_get_class_from_id(long id) {
    return rb_funcall(rb_const_get(rb_cObject, rb_intern("ObjectSpace")), rb_intern("_id2ref"), 1, rb_int2inum(id));
}

int nodeRb_handle_error(int e) {
    if (e) {
        uv_err_t error = uv_last_error();
        fprintf(stderr, "%s\n", uv_strerror(error));
        return 1;
    } else {
        return 0;
    }
};

void nodeRb_next_tick(uv_idle_t* handle, int status) {
    uv_idle_stop(handle);
    free(handle);
    nodeRbNextTickStatus = 0;
    rb_funcall(nodeRb, rb_intern("next_tick_execute"), 0);
}

VALUE nodeRb_nextTick(VALUE self) {
    if(nodeRbNextTickStatus == 0){
        uv_idle_t* handle = malloc(sizeof(uv_idle_t));
        uv_idle_init(handle);
        nodeRbNextTickStatus = 1;
        uv_idle_start(handle, nodeRb_next_tick);
    }
}

VALUE nodeRb_start(VALUE self) {
    // Let the loop run
    uv_ref();
    // Run Ruby
    if (rb_block_given_p()) {
        rb_yield(Qnil);
    }
    // Start loop
    uv_run();
};

void nodeRb_after_write(uv_write_t* req, int status) {
    write_req_t* wr;

    if (status) {
        uv_err_t err = uv_last_error();
        fprintf(stderr, "uv_write error: %s\n", uv_strerror(err));
        return;
    }

    wr = (write_req_t*) req;
    free(wr);

}

VALUE nodeRb_send_data(VALUE self, VALUE data) {
    write_req_t* wr = malloc(sizeof (write_req_t));
    wr->buf.base = rb_string_value_cstr(&data);
    wr->buf.len = RSTRING_LEN(data);
    VALUE rhandler = rb_iv_get(self, "@_handle");
    uv_stream_t *handle;
    Data_Get_Struct(rhandler, uv_stream_t, handle);
    uv_write(&wr->req, handle, &wr->buf, 1, nodeRb_after_write);
}

void nodeRb_on_close(uv_handle_t* client) {
    nodeRb_client *client_data = client->data;
    VALUE object = nodeRb_get_class_from_id(client_data->connection);
    rb_funcall(object, rb_intern("on_close"), 0, 0);
    nodeRb_unregister_instance(nodeRb, object);
    rb_iv_set(object, "@_handle", Qnil);
    free(client_data);
    free(client);
    client_data = NULL;
}

void nodeRb_on_shutdown(uv_shutdown_t* req, int status) {
    uv_close((uv_handle_t*) req->handle, nodeRb_on_close);
    free(req);
}

VALUE nodeRb_close_connection(VALUE self) {
    uv_shutdown_t* req = malloc(sizeof (uv_shutdown_t));
    VALUE rhandler = rb_iv_get(self, "@_handle");
    uv_stream_t *handle;
    Data_Get_Struct(rhandler, uv_stream_t, handle);
    uv_shutdown(req, handle, nodeRb_on_shutdown);
}

void nodeRb_on_read(uv_stream_t* client, ssize_t nread, uv_buf_t buf) {
    nodeRb_client *client_data = client->data;
    if (nread < 0) {
        if (buf.base) {
            free(buf.base);
        }
        return;
    }

    if (nread == 0) {
        free(buf.base);
        return;
    }
    rb_funcall(nodeRb_get_class_from_id(client_data->connection), rb_intern("on_data"), 1, rb_str_new2(buf.base));
    free(buf.base);
}

uv_buf_t nodeRb_read_alloc(uv_stream_t* handle, size_t suggested_size) {
    uv_buf_t buf;
    buf.base = (char*) malloc(suggested_size);
    buf.len = suggested_size;
    return buf;
}

void nodeRb_on_server_connect(uv_stream_t* server, int status) {
    nodeRb_client *client_data = malloc(sizeof (nodeRb_client));
    uv_stream_t *client = malloc(sizeof (uv_tcp_t));
    uv_tcp_init((uv_tcp_t*) client);
    int r = uv_accept(server, client);
    if (nodeRb_handle_error(r)) return;
    VALUE clazz = rb_path2class(server->data);
    VALUE obj = rb_funcall(clazz, rb_intern("new"), 0, 0);
    nodeRb_register_instance(nodeRb, obj);
    rb_funcall(obj, rb_intern("on_connect"), 0, 0);
    rb_iv_set(obj, "@_handle", Data_Wrap_Struct(nodeRbPointer, 0, NULL, client));
    long id = rb_num2long(rb_obj_id(obj));
    client_data->connection = id;
    client->data = client_data;
    uv_read_start(client, nodeRb_read_alloc, nodeRb_on_read);
}

VALUE nodeRb_startServer(VALUE self, VALUE address, VALUE port, VALUE clazz) {
    uv_tcp_t *server = malloc(sizeof (uv_tcp_t));
    uv_tcp_init(server);
    struct sockaddr_in socket = uv_ip4_addr(rb_string_value_cstr(&address), (int) rb_num2long(port));
    int r = uv_tcp_bind(server, socket);
    if (nodeRb_handle_error(r)) return Qnil;
    r = uv_listen((uv_stream_t*) server, 128, nodeRb_on_server_connect);
    if (nodeRb_handle_error(r)) return Qnil;
    VALUE name = rb_class_name(clazz);
    server->data = rb_string_value_cstr(&name);
};

void nodeRb_on_client_connect(uv_connect_t* client, int status) {
    nodeRb_client *client_data = client->handle->data;
    VALUE obj = nodeRb_get_class_from_id(client_data->connection);
    rb_iv_set(obj, "@_handle", Data_Wrap_Struct(nodeRbPointer, 0, NULL, client->handle));
    rb_funcall(obj, rb_intern("on_connect"), 0, 0);
    uv_read_start((uv_stream_t*) client->handle, nodeRb_read_alloc, nodeRb_on_read);
}

VALUE nodeRb_startClient(VALUE self, VALUE address, VALUE port, VALUE clazz) {
    uv_tcp_t *handle = malloc(sizeof (uv_tcp_t));
    uv_tcp_init(handle);
    uv_connect_t *connect = malloc(sizeof (uv_connect_t));
    struct sockaddr_in socket = uv_ip4_addr(rb_string_value_cstr(&address), (int) rb_num2long(port));
    int r = uv_tcp_connect(connect, handle, socket, nodeRb_on_client_connect);
    if (nodeRb_handle_error(r)) return Qnil;
    nodeRb_client *client_data = malloc(sizeof (nodeRb_client));
    VALUE obj = clazz;
    long id = rb_num2long(rb_obj_id(obj));
    client_data->connection = id;
    handle->data = client_data;
};

void nodeRb_process_close(uv_handle_t* handle) {

}

void nodeRb_process_write_after(uv_write_t* req, int status) {

}

VALUE nodeRb_process_write(VALUE self, VALUE data) {
    uv_process_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_process_t, handle);
    uv_write_t write_req;
    uv_buf_t buf;
    char* buffer = rb_string_value_cstr(&data);
    buf.base = buffer;
    buf.len = sizeof (buffer);
    nodeRb_process_handle* nhandle = (nodeRb_process_handle*) handle->data;
    uv_write(&write_req, (uv_stream_t*) nhandle->stdin, &buf, 1, nodeRb_process_write_after);
}

void nodeRb_process_stdout_read(uv_stream_t* handle, ssize_t nread, uv_buf_t buf) {
    uv_err_t err = uv_last_error();
    long id = ((nodeRb_process_handle*) handle->data)->target;
    VALUE clazz = nodeRb_get_class_from_id(id);
    if (nread > 0) {
        rb_funcall(clazz, rb_intern("on_stdout"), 1, rb_str_new(buf.base, nread));
    }
    if (nread < 0) {
        if (err.code == UV_EOF) {
            //uv_close((uv_handle_t*) handle, nodeRb_process_close);
        }
    }
    free(buf.base);
}

void nodeRb_process_stderr_read(uv_stream_t* handle, ssize_t nread, uv_buf_t buf) {
    uv_err_t err = uv_last_error();
    long id = ((nodeRb_process_handle*) handle->data)->target;
    VALUE clazz = nodeRb_get_class_from_id(id);
    if (nread > 0) {
        rb_funcall(clazz, rb_intern("on_stderr"), 1, rb_str_new(buf.base, nread));
    }
    if (nread < 0) {
        if (err.code == UV_EOF) {
            //uv_close((uv_handle_t*) handle, nodeRb_process_close);
        }
    }
    free(buf.base);
}

VALUE nodeRb_process_kill(VALUE self, VALUE signal) {
    uv_process_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_process_t, handle);
    uv_process_kill(handle, (int) rb_num2int(signal));
}

void nodeRb_process_exit(uv_process_t* handle, int status, int signal) {
    nodeRb_process_handle* nhandle = (nodeRb_process_handle*) handle->data;
    VALUE clazz = nodeRb_get_class_from_id(nhandle->target);
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
    nodeRb_process_handle* nhandle = malloc(sizeof(nodeRb_process_handle));
    nhandle->target = rb_num2long(rb_obj_id(clazz));
    // Be safe of GC
    nodeRb_register_instance(nodeRb, clazz);
    // stdin/out/err
    uv_pipe_t* stdout = malloc(sizeof(uv_pipe_t));
    uv_pipe_t* stdin = malloc(sizeof(uv_pipe_t));
    uv_pipe_t* stderr = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(stdout);
    uv_pipe_init(stdin);
    uv_pipe_init(stderr);
    options.stdout_stream = stdout;
    options.stdin_stream = stdin;
    options.stderr_stream = stderr;
    stdout->data = nhandle;
    stderr->data = nhandle;
    nhandle->stdin = stdin;
    // libuv handle
    uv_process_t* handle = malloc(sizeof(uv_process_t));
    handle->data = nhandle;
    // Save reference into Ruby object
    rb_iv_set(clazz, "@_handle", Data_Wrap_Struct(nodeRbPointer, 0, NULL, handle));
    // spawn process
    uv_spawn(handle, options);
    // Listen to stdout/err
    uv_read_start((uv_stream_t*) stdout, nodeRb_read_alloc, nodeRb_process_stdout_read);
    uv_read_start((uv_stream_t*) stderr, nodeRb_read_alloc, nodeRb_process_stderr_read);
    // call back ruby
    rb_funcall(clazz, rb_intern("on_start"), 0);
};

void Init_noderb_extension() {
    // Initialize
    uv_init();
    // Define module
    nodeRb = rb_define_module("NodeRb");
    nodeRbPointer = rb_define_class_under(nodeRb, "Pointer", rb_cObject);
    nodeRbConnection = rb_define_module_under(nodeRb, "Connection");
    nodeRbProcess = rb_define_module_under(nodeRb, "Process");
    // Define connection methods
    rb_define_method(nodeRbConnection, "write", nodeRb_send_data, 1);
    rb_define_method(nodeRbConnection, "close_connection", nodeRb_close_connection, 0);
    // Define process methods
    rb_define_method(nodeRbProcess, "write", nodeRb_process_write, 1);
    rb_define_method(nodeRbProcess, "kill", nodeRb_process_kill, 1);
    // Run code on next tick
    rb_define_singleton_method(nodeRb, "native_next_tick", nodeRb_nextTick, 0);
    // Define starting methods
    rb_define_singleton_method(nodeRb, "start", nodeRb_start, 0);
    rb_define_singleton_method(nodeRb, "start_server", nodeRb_startServer, 3);
    rb_define_singleton_method(nodeRb, "start_client", nodeRb_startClient, 3);
    rb_define_singleton_method(nodeRb, "start_process", nodeRb_startProcess, 5);
    // Memory management
    rb_iv_set(nodeRb, "@instances", rb_ary_new());
    rb_define_singleton_method(nodeRb, "register_instance", nodeRb_register_instance, 1);
    rb_define_singleton_method(nodeRb, "unregister_instance", nodeRb_unregister_instance, 1);
    // Modules
    nodeRbModules = rb_define_module_under(nodeRb, "Modules");
}