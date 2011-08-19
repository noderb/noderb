#ifndef NODERB_TIMERS_H
#define	NODERB_TIMERS_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_timers_once(VALUE self, VALUE timeout, VALUE repeat, VALUE callback);
VALUE nodeRb_timers_stop(VALUE self);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_TIMERS_H */

