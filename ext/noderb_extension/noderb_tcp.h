#ifndef NODERB_TCP_SERVER_H
#define	NODERB_TCP_SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_tcp_startClient(VALUE self, VALUE address, VALUE port, VALUE clazz);

VALUE nodeRb_tcp_startServer(VALUE self, VALUE address, VALUE port, VALUE clazz);
VALUE nodeRb_tcp_send_data(VALUE self, VALUE data);
VALUE nodeRb_tcp_close_connection(VALUE self);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_TCP_SERVER_H */

