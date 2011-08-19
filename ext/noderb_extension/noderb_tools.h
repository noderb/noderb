#ifndef NODERB_TOOLS_H
#define	NODERB_TOOLS_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_nextTick(VALUE self);
VALUE nodeRb_start(VALUE self);

VALUE nodeRb_register_instance(VALUE instance);
VALUE nodeRb_registerInstance(VALUE self, VALUE instance);
VALUE nodeRb_unregister_instance(VALUE instance);
VALUE nodeRb_unregisterInstance(VALUE self, VALUE instance);

VALUE nodeRb_get_class_from_id(long id);

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

uv_buf_t nodeRb_read_alloc(uv_stream_t* handle, size_t suggested_size);
void nodeRb_read(uv_stream_t* uv_handle, ssize_t nread, uv_buf_t buf);
void nodeRb_write(uv_stream_t* handle, char* data, size_t len);
void nodeRb_after_write(uv_write_t* req, int status);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_TOOLS_H */

