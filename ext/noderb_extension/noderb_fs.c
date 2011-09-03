#include <noderb_common.h>

#include <noderb_fs.h>
#include <noderb_tools.h>
#include <fcntl.h>

typedef struct {
    long target;
} nodeRb_file_handle;

char* buf;

void nodeRb_fs_callback(uv_fs_t* handle){
    // Load data
    nodeRb_file_handle* data = (nodeRb_file_handle*) handle->data;
    VALUE target = nodeRb_get_class_from_id(data->target);
    // Something happened
    switch(handle->fs_type) {
        case UV_FS_OPEN:
            // File opened
            printf(""); // ToDo: Error without this?
            uv_file fd = (uv_file) handle->result;
            rb_iv_set(target, "@_handle", INT2NUM(fd));
            rb_funcall(target, rb_intern("on_open"), 0, 0);
            break;
        case UV_FS_READ:
            // Read data from file
            printf(""); // ToDo: Error without this?
            int size = (int) handle->result;
            rb_funcall(target, rb_intern("on_read"), 1, rb_str_new(buf, size));
            break;
        case UV_FS_WRITE:
            // Data written to file
            rb_funcall(target, rb_intern("on_write"), 0, 0);
            break;
        case UV_FS_CLOSE:
            // File closed
            rb_funcall(target, rb_intern("on_close"), 0, 0);
            nodeRb_unregister_instance(target);
            rb_iv_set(target, "@_handle", Qnil);
            break;
    }
    // Clean memory
    free(handle->data);
    uv_fs_req_cleanup(handle);
}

VALUE nodeRb_fs_read(VALUE self, VALUE sizeToRead, VALUE fromToRead){
    uv_file fd = (int) rb_num2long(rb_iv_get(self, "@_handle"));
    long size = rb_num2long(sizeToRead);
    long from = rb_num2long(fromToRead);
    // Allocate memory
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    // Save Ruby data
    data->target = rb_num2long(rb_obj_id(self));
    // Open file
    buf = (char*) malloc(size);
    uv_fs_read(uv_default_loop(), handle, fd, buf, size, from, nodeRb_fs_callback);
    handle->data = data;
}

VALUE nodeRb_fs_write(VALUE self, VALUE buffer){
    // Read data saved in Ruby
    uv_file fd = (int) rb_num2long(rb_iv_get(self, "@_handle"));
    // Allocate memory
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    // Save Ruby data
    data->target = rb_num2long(rb_obj_id(self));
    // Request file close
    uv_fs_write(uv_default_loop(), handle, fd, rb_string_value_ptr(&buffer), RSTRING_LEN(buffer), 0, nodeRb_fs_callback);
    handle->data = data;
}

VALUE nodeRb_fs_close(VALUE self){
    // Read data saved in Ruby
    uv_file fd = (int) rb_num2long(rb_iv_get(self, "@_handle"));
    // Allocate memory
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    // Save Ruby data
    data->target = rb_num2long(rb_obj_id(self));
    // Request file close
    uv_fs_close(uv_default_loop(), handle, fd, nodeRb_fs_callback);
    handle->data = data;
}

// ToDo: make the system to create file if not exists
VALUE nodeRb_fs_open(VALUE self, VALUE file, VALUE rmode, VALUE rgh){
    int mode = (int) rb_num2long(rmode);
    switch(mode){
        case 0:
            mode = O_RDWR;
            break;
        case 1:
            mode = O_RDONLY;
            break;
        case 2:
            mode = O_WRONLY;
            break;
    }
    // Allocate memory
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    // GC
    nodeRb_register_instance(self);
    // Save Ruby data
    data->target = rb_num2long(rb_obj_id(self));
    // Open file
    uv_fs_open(uv_default_loop(), handle, rb_string_value_cstr(&file), mode, (int) rb_num2long(rgh), nodeRb_fs_callback);
    handle->data = data;
}