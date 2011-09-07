#include <noderb_common.h>

#include <noderb_fs.h>
#include <noderb_tools.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    long target;
    char* buffer;
} nodeRb_file_handle;

void nodeRb_fs_operation_callback(uv_fs_t* handle){
    nodeRb_file_handle* data = (nodeRb_file_handle*) handle->data;
    VALUE target = nodeRb_get_class_from_id(data->target);
    if(handle->result == -1){
        rb_funcall(target, rb_intern("call"), 1, Qnil);
    }else{
        switch(handle->fs_type) {
            case UV_FS_UNLINK:
            case UV_FS_MKDIR:
            case UV_FS_RMDIR:
            case UV_FS_RENAME:
            case UV_FS_CHMOD:
            case UV_FS_UTIME:
            case UV_FS_LINK:
            case UV_FS_SYMLINK:
            case UV_FS_CHOWN:
                rb_funcall(target, rb_intern("call"), 1, Qtrue);
                break;
            case UV_FS_READLINK:
                rb_funcall(target, rb_intern("call"), 1, rb_str_new((char*) handle->ptr, handle->result));
                break;
            case UV_FS_READDIR:
                rb_funcall(target, rb_intern("call"), 1, rb_str_new((char*) handle->ptr, handle->result));
                break;
            case UV_FS_LSTAT:
            case UV_FS_STAT:
                {
                    NODERB_STAT_STRUCT* stats = (NODERB_STAT_STRUCT*) handle->ptr;
                    VALUE values = rb_funcall(rb_path2class("NodeRb::Stat"), rb_intern("new"), 0, 0);

                    rb_iv_set(values, "@device", LONG2NUM(stats->st_dev));
                    rb_iv_set(values, "@inode", LONG2NUM(stats->st_ino));
                    rb_iv_set(values, "@rdevice", LONG2NUM(stats->st_rdev));

                    rb_iv_set(values, "@uid", LONG2NUM(stats->st_uid));
                    rb_iv_set(values, "@gid", LONG2NUM(stats->st_gid));
                    rb_iv_set(values, "@size", LONG2NUM(stats->st_size));
                    rb_iv_set(values, "@links", LONG2NUM(stats->st_nlink));
                    rb_iv_set(values, "@mode", LONG2NUM(stats->st_mode));
                    rb_iv_set(values, "@block_size", LONG2NUM(stats->st_blksize));
                    rb_iv_set(values, "@blocks", LONG2NUM(stats->st_blocks));

                    rb_iv_set(values, "@accessed", LONG2NUM(stats->st_atime));
                    rb_iv_set(values, "@modified", LONG2NUM(stats->st_mtime));
                    rb_iv_set(values, "@changed", LONG2NUM(stats->st_ctime));

                    rb_funcall(target, rb_intern("call"), 1, values);
                }
                break;
        }
    }
    nodeRb_unregister_instance(target);
    free(handle->data);
    uv_fs_req_cleanup(handle);
}

VALUE nodeRb_fs_operation(VALUE self, VALUE roperation, VALUE path, VALUE params, VALUE callback){
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    nodeRb_register_instance(callback);
    data->target = rb_num2long(rb_obj_id(callback));
    long operation = rb_num2long(roperation);
    switch(operation){
        case 0:
            uv_fs_unlink(uv_default_loop(), handle, rb_string_value_cstr(&path), nodeRb_fs_operation_callback);
            break;
        case 1:
            uv_fs_mkdir(uv_default_loop(), handle, rb_string_value_cstr(&path), (int) rb_num2long(params), nodeRb_fs_operation_callback);
            break;
        case 2:
            uv_fs_rmdir(uv_default_loop(), handle, rb_string_value_cstr(&path), nodeRb_fs_operation_callback);
            break;
        case 3:
            uv_fs_readdir(uv_default_loop(), handle, rb_string_value_cstr(&path), (int) rb_num2long(params), nodeRb_fs_operation_callback);
            break;
        case 4:
            uv_fs_stat(uv_default_loop(), handle, rb_string_value_cstr(&path), nodeRb_fs_operation_callback);
            break;
        case 5:
            uv_fs_rename(uv_default_loop(), handle, rb_string_value_cstr(&path), rb_string_value_cstr(&params), nodeRb_fs_operation_callback);
            break;
        case 6:
            uv_fs_chmod(uv_default_loop(), handle, rb_string_value_cstr(&path), (int) rb_num2long(params), nodeRb_fs_operation_callback);
            break;
        case 7:
            uv_fs_utime(uv_default_loop(), handle, rb_string_value_cstr(&path), (int) rb_num2long(rb_ary_entry(params, 0)), (int) rb_num2long(rb_ary_entry(params, 1)), nodeRb_fs_operation_callback);
            break;
        case 8:
            uv_fs_lstat(uv_default_loop(), handle, rb_string_value_cstr(&path), nodeRb_fs_operation_callback);
            break;
        case 9:
            uv_fs_link(uv_default_loop(), handle, rb_string_value_cstr(&path), rb_string_value_cstr(&params), nodeRb_fs_operation_callback);
            break;
        case 10:
            {
                VALUE new_path = rb_ary_entry(params, 0);
                // (int) rb_num2long(rb_ary_entry(params, 1))
                uv_fs_symlink(uv_default_loop(), handle, rb_string_value_cstr(&path), rb_string_value_cstr(&new_path), nodeRb_fs_operation_callback);
            }
            break;
        case 11:
            uv_fs_readlink(uv_default_loop(), handle, rb_string_value_cstr(&path), nodeRb_fs_operation_callback);
            break;
        case 12:
            uv_fs_chown(uv_default_loop(), handle, rb_string_value_cstr(&path), (int) rb_num2long(rb_ary_entry(params, 0)), (int) rb_num2long(rb_ary_entry(params, 1)), nodeRb_fs_operation_callback);
            break;
    }
    handle->data = data;
}

void nodeRb_fs_file_operation_callback(uv_fs_t* handle){
    nodeRb_file_handle* data = (nodeRb_file_handle*) handle->data;
    VALUE target = nodeRb_get_class_from_id(data->target);
    if(handle->result == -1){
        rb_funcall(target, rb_intern("on_error"), 0, 0);
    }else{
        switch(handle->fs_type) {
            case UV_FS_SENDFILE:
                rb_funcall(target, rb_intern("on_sendfile"), 0, 0);
                break;
            case UV_FS_FTRUNCATE:
                rb_funcall(target, rb_intern("on_truncate"), 0, 0);
                break;
            case UV_FS_FUTIME:
                rb_funcall(target, rb_intern("on_utime"), 0, 0);
                break;
            case UV_FS_FCHMOD:
                rb_funcall(target, rb_intern("on_chmod"), 0, 0);
                break;
            case UV_FS_FSYNC:
                rb_funcall(target, rb_intern("on_sync"), 0, 0);
                break;
            case UV_FS_FDATASYNC:
                rb_funcall(target, rb_intern("on_datasync"), 0, 0);
                break;
            case UV_FS_FCHOWN:
                rb_funcall(target, rb_intern("on_chown"), 0, 0);
                break;
            case UV_FS_WRITE:
                rb_funcall(target, rb_intern("on_write"), 0, 0);
                break;
            case UV_FS_FSTAT:
                {
                    NODERB_STAT_STRUCT* stats = (NODERB_STAT_STRUCT*) handle->ptr;
                    VALUE values = rb_funcall(rb_path2class("NodeRb::Stat"), rb_intern("new"), 0, 0);

                    rb_iv_set(values, "@device", LONG2NUM(stats->st_dev));
                    rb_iv_set(values, "@inode", LONG2NUM(stats->st_ino));
                    rb_iv_set(values, "@rdevice", LONG2NUM(stats->st_rdev));

                    rb_iv_set(values, "@uid", LONG2NUM(stats->st_uid));
                    rb_iv_set(values, "@gid", LONG2NUM(stats->st_gid));
                    rb_iv_set(values, "@size", LONG2NUM(stats->st_size));
                    rb_iv_set(values, "@links", LONG2NUM(stats->st_nlink));
                    rb_iv_set(values, "@mode", LONG2NUM(stats->st_mode));
                    rb_iv_set(values, "@block_size", LONG2NUM(stats->st_blksize));
                    rb_iv_set(values, "@blocks", LONG2NUM(stats->st_blocks));

                    rb_iv_set(values, "@accessed", LONG2NUM(stats->st_atime));
                    rb_iv_set(values, "@modified", LONG2NUM(stats->st_mtime));
                    rb_iv_set(values, "@changed", LONG2NUM(stats->st_ctime));

                    rb_funcall(target, rb_intern("on_stat"), 1, values);
                }
                break;
            case UV_FS_OPEN:
                {
                    uv_file fd = (uv_file) handle->result;
                    rb_iv_set(target, "@_handle", INT2NUM(fd));
                    rb_funcall(target, rb_intern("on_open"), 0, 0);
                }
                break;
            case UV_FS_READ:
                {
                    int size = (int) handle->result;
                    char* buf = data->buffer;
                    rb_funcall(target, rb_intern("on_read"), 1, rb_str_new(buf, size));
                    free(buf);
                }
                break;
            case UV_FS_CLOSE:
                rb_funcall(target, rb_intern("on_close"), 0, 0);
                nodeRb_unregister_instance(target);
                rb_iv_set(target, "@_handle", Qnil);
                break;
        }
    }
    free(handle->data);
    uv_fs_req_cleanup(handle);
}

VALUE nodeRb_fs_file_operation(VALUE self, VALUE roperation, VALUE params){
    long operation = rb_num2long(roperation);
    uv_file fd;
    if(operation > 0){
        fd = (int) rb_num2long(rb_iv_get(self, "@_handle"));
    }
    // Allocate memory
    uv_fs_t* handle = malloc(sizeof(uv_fs_t));
    nodeRb_file_handle* data = malloc(sizeof(nodeRb_file_handle));
    // Save Ruby data
    data->target = rb_num2long(rb_obj_id(self));
    // Open file
    switch(operation){
        case 0:
            {
                int mode = (int) rb_num2long(rb_ary_entry(params,1));
                switch(mode){
                    case 0:
                        mode = O_RDWR | O_CREAT;
                        break;
                    case 1:
                        mode = O_RDONLY;
                        break;
                    case 2:
                        mode = O_WRONLY | O_CREAT;
                        break;
                }
                VALUE file = rb_ary_entry(params,0);
                uv_fs_open(uv_default_loop(), handle, rb_string_value_cstr(&file), mode, (int) rb_num2long(rb_ary_entry(params,2)), nodeRb_fs_file_operation_callback);
            }
            break;
        case 1:
            {
                long size = rb_num2long(rb_ary_entry(params,0));
                long offset = rb_num2long(rb_ary_entry(params,1));
                char* buf = (char*) malloc(size);
                data->buffer = buf;
                uv_fs_read(uv_default_loop(), handle, fd, buf, size, offset, nodeRb_fs_file_operation_callback);
            }
            break;
        case 2:
            uv_fs_write(uv_default_loop(), handle, fd, rb_string_value_ptr(&params), RSTRING_LEN(params), 0, nodeRb_fs_file_operation_callback);
            break;
        case 3:
            uv_fs_fstat(uv_default_loop(), handle, fd, nodeRb_fs_file_operation_callback);
            break;
        case 4:
            uv_fs_fsync(uv_default_loop(), handle, fd, nodeRb_fs_file_operation_callback);
            break;
        case 5:
            uv_fs_fdatasync(uv_default_loop(), handle, fd, nodeRb_fs_file_operation_callback);
            break;
        case 6:
            uv_fs_ftruncate(uv_default_loop(), handle, fd, (int) rb_num2long(params), nodeRb_fs_file_operation_callback);
            break;
        case 7:
            uv_fs_sendfile(uv_default_loop(), handle, fd, (int) rb_num2long(rb_ary_entry(params,0)), (int) rb_num2long(rb_ary_entry(params,1)), (int) rb_num2long(rb_ary_entry(params,2)), nodeRb_fs_file_operation_callback);
            break;
        case 8:
            uv_fs_futime(uv_default_loop(), handle, fd, (int) rb_num2long(rb_ary_entry(params,0)), (int) rb_num2long(rb_ary_entry(params,1)), nodeRb_fs_file_operation_callback);
            break;
        case 9:
            uv_fs_fchmod(uv_default_loop(), handle, fd, (int) rb_num2long(params), nodeRb_fs_file_operation_callback);
            break;
        case 10:
            uv_fs_fchown(uv_default_loop(), handle, fd, (int) rb_num2long(rb_ary_entry(params,0)), (int) rb_num2long(rb_ary_entry(params,1)), nodeRb_fs_file_operation_callback);
            break;
        case 11:
            uv_fs_close(uv_default_loop(), handle, fd, nodeRb_fs_file_operation_callback);
            break;
    }
    handle->data = data;
}