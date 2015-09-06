#ifndef termio_raw_h__
#define termio_raw_h__
/*
 * termio_raw.h
 *
 * Copyright (c) 2015, Matthew Madison.
 * Distributed under license, see LICENSE for more information.
 */

struct termio_ctx_s;
typedef struct termio_ctx_s *termio_ctx_t;

struct termio_termsize_s {
	unsigned term_rows;
	unsigned term_cols;
};

termio_ctx_t termio_init(int ttyfd);
int termio_termsize_setup(termio_ctx_t ctx, struct termio_termsize_s *sz);
void termio_finish(termio_ctx_t ctx);

#endif /* termio_raw_h__ */
