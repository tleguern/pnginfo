/*
 * Copyright (c) 2022 Tristan Le Guern <tleguern@bouledef.eu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <arpa/inet.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lgpng.h"

bool
lgpng_is_stream_png(FILE *src)
{
	char sig[8] = {  0,  0,  0,  0,  0,  0,  0,  0};

	if (sizeof(sig) != fread(sig, 1, sizeof(sig), src)) {
		return(false);
	}
	if (memcmp(sig, png_sig, sizeof(sig)) == 0)
		return(true);
	return(false);
}

int
lgpng_get_next_chunk_from_stream(FILE *src, struct unknown_chunk *dst, uint8_t **data)
{
	/* Read the first four bytes to gather the length of the data part */
	if (1 != fread(&(dst->length), 4, 1, src)) {
		fprintf(stderr, "Not enough data to read chunk's length\n");
		return(-1);
	}
	dst->length = ntohl(dst->length);
	if (dst->length > INT32_MAX) {
		fprintf(stderr, "Chunk length is too big (%d)\n", dst->length);
		return(-1);
	}
	/* Read the chunk type */
	if (4 != fread(&(dst->type), 1, 4, src)) {
		fprintf(stderr, "Not enough data to read chunk's type\n");
		return(-1);
	}
	dst->type[4] = '\0';
	for (size_t i = 0; i < 4; i++) {
		if (isalpha(dst->type[i]) == 0) {
			fprintf(stderr, "Invalid chunk type\n");
			return(-1);
		}
	}
	/* Read the chunk data */
	if (0 != dst->length) {
		if (NULL == ((*data) = malloc(dst->length + 1))) {
			fprintf(stderr, "malloc(dst->length)\n");
			return(-1);
		}
		if (dst->length != fread((*data), 1, dst->length, src)) {
			fprintf(stderr, "Not enough data to read chunk's data\n");
			free(*data);
			(*data) = NULL;
			return(-1);
		}
		(*data)[dst->length] = '\0';
	}
	/* Read the CRC */
	if (1 != fread(&(dst->crc), 4, 1, src)) {
		fprintf(stderr, "Not enough data to read chunk's CRC\n");
		free(*data);
		(*data) = NULL;
		return(-1);
	}
	dst->crc = ntohl(dst->crc);
	return(0);
}

