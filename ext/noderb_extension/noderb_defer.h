#ifndef NODERB_DEFER_H
#define	NODERB_DEFER_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_defer(VALUE self, VALUE worker, VALUE callback);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_DEFER_H */

