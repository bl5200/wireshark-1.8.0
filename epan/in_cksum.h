/* in_cksum.h
 * Declaration of  Internet checksum routine.
 *
 * $Id: in_cksum.h 38995 2011-09-14 14:53:49Z stig $
 */

#ifndef __IN_CKSUM_H__
#define __IN_CKSUM_H__

typedef struct {
	const guint8 *ptr;
	int	len;
} vec_t;

extern int in_cksum(const vec_t *vec, int veclen);

extern guint16 in_cksum_shouldbe(guint16 sum, guint16 computed_sum);

#endif /* __IN_CKSUM_H__ */
