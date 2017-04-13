/*
 * Copyright 2013 Ayla Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Ayla Networks, Inc.
 */

/*
 * Builds under Linux, MacOSX, win32 visual c++ 
 */
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef WIN32
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#else
#include <io.h>
#include <sys/stat.h>
#endif

#include "crc.h"

static char *progname;
static unsigned char image[1024 * 1024];

#ifdef WIN32
#define htons(a) ((((a) << 8) & 0xff00) | (((a) >> 8) & 0xff))
#define htonl(a)							\
	((((a) << 24) & 0xff000000) | (((a) << 8) & 0xff0000) |		\
	    (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))
#define open _open
#define read _read
#define write _write
#define FILE_PERM (_S_IREAD | _S_IWRITE)
#else
#ifndef O_BINARY
#define O_BINARY 0
#endif

#define FILE_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif

struct {
	unsigned int off;
	unsigned int magic;
	char *name;
} targets[] = {
	{ 0x130, 0xbfa43640, "stm32vl" },
	{ 0x188, 0xbfa43641, "stm32f3" },
	{ 0x150, 0xbfa43642, "stm32f1" }
};

void
usage(int code)
{
	int i;

	printf("usage: %s -i <in.bin> -o <out.img> -t <target>\n", progname);
	printf("       valid targets :");
	for (i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
		printf("%s ", targets[i].name);
	}
	printf("\n");
	exit(code);
}

int
target_data(char *tgt, unsigned int *off, unsigned int *magic)
{
	int i;

	for (i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
		if (!strcmp(tgt, targets[i].name)) {
			*off = targets[i].off;
			*magic = targets[i].magic;
			return 0;
		}
	}
	return 1;
}

int
main(int argc, char **argv)
{
	int ilen, i;
	char *ifile = NULL, *ofile = NULL, *target = NULL;
	int ifd, ofd;
	unsigned int hdr_off, hdr_magic;
	unsigned int *p32;
	unsigned short *p16;
	u16 crc;

	progname = argv[0];
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i")) {
			i++;
			if (i >= argc) {
				usage(1);
			}
			ifile = argv[i];
		} else if (!strcmp(argv[i], "-o")) {
			i++;
			if (i >= argc) {
				usage(1);
			}
			ofile = argv[i];
		} else if (!strcmp(argv[i], "-t")) {
			i++;
			if (i >= argc) {
				usage(1);
			}
			target = argv[i];
		} else {
			usage(0);
		}
	}
	if (!ifile || !ofile || !target) {
		usage(1);
	}
	ifd = open(ifile, O_RDONLY | O_BINARY);
	if (ifd < 0) {
		perror("open() failed for input file");
		exit(1);
	}
	ofd = open(ofile, O_CREAT | O_WRONLY | O_BINARY, FILE_PERM);
	if (ofd < 0) {
		perror("open() failed for output file");
		exit(1);
	}
	ilen = read(ifd, image, sizeof(image));
	if (ilen < 0) {
		perror("read() of input file failed");
		exit(1);
	}

	if (target_data(target, &hdr_off, &hdr_magic)) {
		fprintf(stderr, "Invalid target %s\n", target);
		usage(1);
	}

	/*
	 * struct image at offset hdr_off.
	 * See flash_layout.h for details. Can type cast a pointer,
	 * as windows pads the structure in a way that firmware doesn't
	 * understand.
	 */
	/*
	 * hdr->i_magic = htonl(IMAGE_MAGIC);
	 */
	p32 = (unsigned int *)&image[hdr_off];
	*p32 = htonl(hdr_magic);

	/*
	 * hdr->i_crc = 0;
	 */
	p16 = (unsigned short *)&image[hdr_off + sizeof(unsigned int)];
	*p16 = 0;

	/*
	 * hdr->i_len = htons(ilen);
	 */
	p16++;
	*p16 = htons(ilen);
	printf("version %s\n", (char *)(p16 + 1));

	crc = crc16(image, ilen, CRC16_INIT);
	p16--;
	*p16 = htons(crc);
	if (write(ofd, image, ilen) != ilen) {
		perror("write() failed");
	}

	exit(0);
}
