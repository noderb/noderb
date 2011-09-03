#ifndef NODERB_FS_H
#define	NODERB_FS_H

#ifdef	__cplusplus
extern "C" {
#endif

VALUE nodeRb_fs_open(VALUE self, VALUE file, VALUE mode, VALUE rgh);
VALUE nodeRb_fs_read(VALUE self, VALUE size, VALUE fromToRead);
VALUE nodeRb_fs_write(VALUE self, VALUE data);
VALUE nodeRb_fs_close(VALUE self);

#ifdef	__cplusplus
}
#endif

#endif	/* NODERB_FS_H */

