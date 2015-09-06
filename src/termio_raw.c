/*
 * termio_raw.c
 *
 * Terminal I/O routines.
 *
 * Copyright (c) 2015, Matthew Madison.
 * Distributed under license, see LICENSE for more information.
 */

#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "termio_raw.h"

#define ANSI_CSI_7BIT "\033["

static const char termsize_query[] = ANSI_CSI_7BIT "18t";
static size_t termsize_query_len = sizeof(termsize_query);

struct termio_ctx_s {
	int fd;
	fd_set readfds;
	struct timeval tv;
	struct termios saved_termios;
};

/*
 * timed_readchar
 *
 * Reads one character from the terminal, with timeout.
 */
static int
timed_readchar (termio_ctx_t ctx, unsigned char *cp)
{
	unsigned char chbuf;
	int n;

	FD_ZERO(&ctx->readfds);
	FD_SET(ctx->fd, &ctx->readfds);
	n = select(ctx->fd + 1, &ctx->readfds, NULL, NULL, &ctx->tv);
	if (n < 0)
		return -1;
	if (!FD_ISSET(ctx->fd, &ctx->readfds))
		return -1;
	n = read(ctx->fd, &chbuf, sizeof(chbuf));
	if (n != sizeof(chbuf))
		return -1;	
	*cp = chbuf;
	return 0;

} /* timed_readchar */

/*
 * Read a CSI (either 7-bit or 8-bit) from the terminal.
 *
 * Returns 1 if match, 0 otherwise
 */
static int
expect_csi (termio_ctx_t ctx)
{
	unsigned char ch;

	if (timed_readchar(ctx, &ch) < 0)
		return 0;
	if (ch ==  0x9B) /* 8-bit CSI */
		return 1;
	if (ch != 27) /* ESC */
		return 0;
	if (timed_readchar(ctx, &ch) < 0)
		return 0;
	return (ch == '[');

} /* expect_csi */

/*
 * Read and match characters read from the terminal
 *
 * Returns 1 if match, 0 otherwise
 */
static int
expect (termio_ctx_t ctx, const unsigned char *match, size_t matchlen)
{
	unsigned char ch;
	const unsigned char *cp;
	size_t i;

	for (cp = match, i = 0; i < matchlen; cp++, i++)
		if (timed_readchar(ctx, &ch) < 0 || ch != *cp)
			return 0;
	return 1;

} /* expect */

/*
 * termio_init
 *
 * Set up for raw terminal I/O on the specified file
 * descriptor.
 *
 * Returns NULL if the file descriptor is not a channel
 * to a terminal, or due to some other setup failure.
 *
 * Returns non-NULL context handle when successful.
 * Callers must not direct I/O to the file descriptor
 * until termio_close() has been called to restore the
 * terminal to normal I/O.
 */
termio_ctx_t
termio_init (int ttyfd)
{
	struct termio_ctx_s *ctx;
	struct termios tios;

	if (!isatty(ttyfd))
		return NULL;
	if (ioctl(ttyfd, TCGETS, &tios) < 0)
		return NULL;
	ctx = malloc(sizeof(struct termio_ctx_s));
	if (ctx == NULL)
		return NULL;
	memset(ctx, 0, sizeof(struct termio_ctx_s));
	memcpy(&ctx->saved_termios, &tios, sizeof(ctx->saved_termios));
	tios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	tios.c_oflag &= ~OPOST;
	tios.c_lflag &= ~(ICANON|ECHO|ISIG|IEXTEN);
	tios.c_cflag &= ~(CSIZE|PARENB);
	tios.c_cflag |= CS8;
	if (ioctl(ttyfd, TCSETS, &tios) < 0) {
		free(ctx);
		ctx = NULL;
	} else {
		ctx->fd = ttyfd;
		ctx->tv.tv_sec = 1;
	}
	return ctx;

} /* termio_init */

/*
 * termio_termsize_setup
 *
 * Queries the terminal for its size (that is, its addressable
 * number of rows and columns, excluding any status rows that
 * might be outside the addressable window) and informs the
 * tty driver of that size.  Optionally returns the size information
 * to the caller as well.
 *
 * Returns:
 *   < 0 - error occurred
 *     0 - success
 */
int
termio_termsize_setup (termio_ctx_t ctx, struct termio_termsize_s *sz)
{
	struct termio_termsize_s tsize;
	struct winsize winsz;
	enum {
		STATE_ROWS,
		STATE_COLS,
		STATE_DONE
	} curstate;
	unsigned char ch;
	static unsigned char response[] = { '8', ';' };

	if (write(ctx->fd, termsize_query, termsize_query_len) != termsize_query_len)
		return -1;
	tcdrain(ctx->fd);
	if (!expect_csi(ctx))
		return -1;
	if (!expect(ctx, response, sizeof(response)))
		return -1;
	tsize.term_rows = tsize.term_cols = 0;
	curstate = STATE_ROWS;
	while (timed_readchar(ctx, &ch) >= 0) {
		if (curstate == STATE_ROWS) {
			if (ch == ';') {
				curstate = STATE_COLS;
				continue;
			}
			if (ch < '0' || ch > '9')
				break;
			tsize.term_rows = tsize.term_rows * 10 + (ch - '0');
		} else if (curstate == STATE_COLS) {
			if (ch == 't') {
				curstate = STATE_DONE;
				break;
			}
			if (ch < '0' || ch > '9')
				return -1;
			tsize.term_cols = tsize.term_cols * 10 + (ch - '0');
		}
	}
	if (curstate != STATE_DONE)
		return -1;
	memset(&winsz, 0, sizeof(winsz));
	winsz.ws_row = tsize.term_rows;
	winsz.ws_col = tsize.term_cols;
	ioctl(ctx->fd, TIOCSWINSZ, &winsz);
	if (sz != NULL)
		memcpy(sz, &tsize, sizeof(struct termio_termsize_s));
	return 0;

} /* termio_termsize_setup */

/*
 * termio_finish
 *
 */
void
termio_finish (termio_ctx_t ctx)
{
	ioctl(ctx->fd, TCSETS, &ctx->saved_termios);
	free(ctx);

} /* termio_finish */
