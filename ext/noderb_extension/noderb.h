#ifndef NODERB_H
#define	NODERB_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_get_nodeRb_module();
VALUE nodeRb_get_nodeRb_pointer();

void Init_noderb_extension();

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_H */

