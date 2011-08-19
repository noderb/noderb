#ifndef NODERB_PROCESS_H
#define	NODERB_PROCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_startProcess(VALUE self, VALUE executable, VALUE arguments, VALUE environment, VALUE directory, VALUE clazz);
VALUE nodeRb_process_write(VALUE self, VALUE data);
VALUE nodeRb_process_kill(VALUE self, VALUE signal);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_PROCESS_H */

