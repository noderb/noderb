#include <noderb_common.h>
#include <noderb_tcp.h>

#include <noderb.h>
#include <noderb_tools.h>

typedef struct {
    NODERB_BASIC_HANDLE
} nodeRb_client;

/*
    NodeRb TCP
*/

void nodeRb_tcp_on_close(uv_handle_t* client) {
    // Read data saved in Ruby
    nodeRb_client *client_data = client->data;
    VALUE object = nodeRb_get_class_from_id(client_data->target);
    // Call callback for connection close
    rb_funcall(object, rb_intern("on_close"), 0, 0);
    // Allow GC of handler instance
    nodeRb_unregister_instance(object);
    rb_iv_set(object, "@_handle", Qnil);
    rb_iv_set(object, "@_proxy_target", Qnil);
    // Clean up some memory
    free(client_data);
    free(client);
}

void nodeRb_tcp_on_shutdown(uv_shutdown_t* req, int status) {
    // When connection shutdown, close it
    uv_close((uv_handle_t*) req->handle, nodeRb_tcp_on_close);
    free(req);
}

VALUE nodeRb_tcp_close_connection(VALUE self) {
    // Allocate new shutdown request
    uv_shutdown_t* req = malloc(sizeof (uv_shutdown_t));
    // Read data saved in Ruby
    VALUE rhandler = rb_iv_get(self, "@_handle");
    uv_stream_t *handle;
    Data_Get_Struct(rhandler, uv_stream_t, handle);
    // Request shutdown of stream
    uv_shutdown(req, handle, nodeRb_tcp_on_shutdown);
}

VALUE nodeRb_tcp_send_data(VALUE self, VALUE data) {
    // Read data saved in Ruby
    uv_stream_t *handle;
    Data_Get_Struct(rb_iv_get(self, "@_handle"), uv_stream_t, handle);
    // Request write to stream
    nodeRb_write(handle, rb_string_value_ptr(&data), RSTRING_LEN(data));
}

/*
    NodeRb TCP client
*/

void nodeRb_tcp_on_client_connect(uv_connect_t* client, int status) {
    nodeRb_client *client_data = client->handle->data;
    VALUE obj = nodeRb_get_class_from_id(client_data->target);
    rb_iv_set(obj, "@_handle", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, client->handle));
    rb_iv_set(obj, "@_proxy_target", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, client->handle));
    rb_funcall(obj, rb_intern("on_connect"), 0, 0);
    uv_read_start((uv_stream_t*) client->handle, nodeRb_read_alloc, nodeRb_read);
}

VALUE nodeRb_tcp_startClient(VALUE self, VALUE address, VALUE port, VALUE clazz) {
    // Allocate memory for connection
    uv_tcp_t *handle = malloc(sizeof (uv_tcp_t));
    uv_tcp_init(uv_default_loop(), handle);
    uv_connect_t *connect = malloc(sizeof (uv_connect_t));
    // Translate IP address
    struct sockaddr_in socket = uv_ip4_addr(rb_string_value_cstr(&address), (int) rb_num2long(port));
    // Save client data
    nodeRb_client *client_data = malloc(sizeof (nodeRb_client));
    client_data->target = rb_num2long(rb_obj_id(clazz));
    client_data->target_callback = (char*) "on_data";
    handle->data = client_data;
    // Open socket
    int r = uv_tcp_connect(connect, handle, socket, nodeRb_tcp_on_client_connect);
    if (nodeRb_handle_error(r)) return Qnil;
};

/*
    NodeRb TCP server
*/

void nodeRb_tcp_on_server_connect(uv_stream_t* server, int status) {
    // Allocate memory for client data
    nodeRb_client *client_data = malloc(sizeof (nodeRb_client));
    // Allocate for client
    uv_stream_t *client = malloc(sizeof (uv_tcp_t));
    uv_tcp_init(uv_default_loop(), (uv_tcp_t*) client);
    // Accept connection
    int r = uv_accept(server, client);
    if (nodeRb_handle_error(r)) return;
    // Initialise new instance of handler class
    VALUE clazz = rb_path2class(server->data);
    VALUE obj = rb_funcall(clazz, rb_intern("new"), 0, 0);
    // Be safe from GC
    nodeRb_register_instance(obj);
    // Save client data with this connection handler instance
    rb_iv_set(obj, "@_handle", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, client));
    rb_iv_set(obj, "@_proxy_target", Data_Wrap_Struct(nodeRb_get_nodeRb_pointer(), 0, NULL, client));
    // Get object id of handler instance
    client_data->target = rb_num2long(rb_obj_id(obj));
    client_data->target_callback = (char*) "on_data";
    client->data = client_data;
    // Call callback
    rb_funcall(obj, rb_intern("on_connect"), 0, 0);
    // Listen for incoming data
    uv_read_start(client, nodeRb_read_alloc, nodeRb_read);
}

VALUE nodeRb_tcp_startServer(VALUE self, VALUE address, VALUE port, VALUE clazz) {
    // Allocate for server
    uv_tcp_t *server = malloc(sizeof (uv_tcp_t));
    uv_tcp_init(uv_default_loop(), server);
    // Translate IP address
    struct sockaddr_in socket = uv_ip4_addr(rb_string_value_cstr(&address), (int) rb_num2long(port));
    // Bind on socket
    int r = uv_tcp_bind(server, socket);
    if (nodeRb_handle_error(r)) return Qnil;
    // Listen on server
    r = uv_listen((uv_stream_t*) server, 128, nodeRb_tcp_on_server_connect);
    if (nodeRb_handle_error(r)) return Qnil;
    // Save information for server
    VALUE name = rb_class_name(clazz);
    server->data = rb_string_value_cstr(&name);
};
