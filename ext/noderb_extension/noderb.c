#include <noderb_common.h>
#include <noderb.h>

#include <noderb_process.h>
#include <noderb_tcp.h>
#include <noderb_dns.h>
#include <noderb_timers.h>
#include <noderb_tools.h>
//#include <noderb_defer.h>
#include <noderb_fs.h>

// Core NodeRb module
VALUE nodeRb;
// Pointer for saving native data
VALUE nodeRbPointer;

// Core module of NodeRb
VALUE nodeRb_get_nodeRb_module(){
    return nodeRb;
}

// NodeRb pointer is used for wrapping native structs into Ruby
VALUE nodeRb_get_nodeRb_pointer(){
    return nodeRbPointer;
}

void Init_noderb_extension() {
    // Define module
    nodeRb = rb_define_module("NodeRb");
    // Define native pointer
    nodeRbPointer = rb_define_class_under(nodeRb, "NativePointer", rb_cObject);
    // Define handlers of events
    VALUE nodeRbConnection = rb_define_module_under(nodeRb, "Connection");
    VALUE nodeRbProcess = rb_define_module_under(nodeRb, "Process");
    // Define connection methods
    rb_define_method(nodeRbConnection, "write_data", nodeRb_tcp_send_data, 1);
    rb_define_method(nodeRbConnection, "close_connection", nodeRb_tcp_close_connection, 0);
    // Define process methods
    rb_define_method(nodeRbProcess, "write_data", nodeRb_process_write, 1);
    rb_define_method(nodeRbProcess, "kill_process", nodeRb_process_kill, 1);
    // Define utility methods
    rb_define_singleton_method(nodeRb, "next_tick_native", nodeRb_nextTick, 0);
    rb_define_singleton_method(nodeRb, "start_proxy", nodeRb_startProxy, 2);
    rb_define_singleton_method(nodeRb, "stop_proxy", nodeRb_stopProxy, 1);
    // DNS related methods
    rb_define_singleton_method(nodeRb, "resolve_native", nodeRb_dns_resolve, 2);
    // Timer related functions
    rb_define_singleton_method(nodeRb, "once_native", nodeRb_timers_once, 3);
    VALUE timer = rb_define_module_under(nodeRb, "Timer");
    rb_define_method(timer, "stop_timer", nodeRb_timers_stop, 0);
    // Defer functions
    //rb_define_singleton_method(nodeRb, "defer_native", nodeRb_defer, 2);
    // Filesystem functions
    VALUE nodeRbFile = rb_define_module_under(nodeRb, "File");
    VALUE nodeRbFS = rb_define_class_under(nodeRb, "FileSystem", rb_cObject);
    // File operations
    rb_define_method(nodeRbFile, "operation_native", nodeRb_fs_file_operation, 2);
    // Filesystem operations
    rb_define_singleton_method(nodeRbFS, "fs_native", nodeRb_fs_operation, 4);
    // Define starting methods
    rb_define_singleton_method(nodeRb, "start", nodeRb_start, 0);
    rb_define_singleton_method(nodeRb, "start_server_native", nodeRb_tcp_startServer, 3);
    rb_define_singleton_method(nodeRb, "start_client_native", nodeRb_tcp_startClient, 3);
    rb_define_singleton_method(nodeRb, "start_process", nodeRb_startProcess, 5);
    // Memory management
    rb_iv_set(nodeRb, "@instances", rb_ary_new());
    rb_define_singleton_method(nodeRb, "register_instance", nodeRb_registerInstance, 1);
    rb_define_singleton_method(nodeRb, "unregister_instance", nodeRb_unregisterInstance, 1);
    // Modules
    VALUE nodeRbModules = rb_define_module_under(nodeRb, "Modules");
}