/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * $File$ - SunOs 4.1.3/Sparc: access single sided disk
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * revision history
 *  23.05.2002  0.02.00 JH  FAST1.0.1 code import: KR -> ANSI, restructuring
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

unsigned        sides;
unsigned        tracks;
unsigned        sectors;
unsigned        secsize;

int             main (argc, argv)
    int             argc;
    char          **argv;
{
    int             fd, i;
    FILE           *f;
    unsigned char  *buf;
    if (argc != 2) {
        fprintf (stderr, "Usage: %s <outfile>\n", argv[0]);
        exit (1);
    }
    if (NULL == (f = fopen (argv[1], "wb"))) {
        perror ("open");
        exit (1);
    }
#ifdef sun
    fd = open ("/dev/rfd0", O_RDONLY);
#else
    fd = open ("/dev/fd0", O_RDONLY);
#endif
    if (fd < 0) {
        perror ("open");
        exit (1);
    }
    buf = (unsigned char *) malloc (512);

    if (512 != read (fd, buf, 512)) {
        perror ("read");
        exit (1);
    }
    sides = (unsigned) *(buf + 26);
    sectors = (unsigned) *(buf + 24);
    secsize = (unsigned) (*(buf + 12) << 8) | *(buf + 11);
    tracks = (unsigned) ((*(buf + 20) << 8) | *(buf + 19)) / (sectors * sides);
    fprintf (stderr,
     "Disk format: %d-sided, %d tracks, %d sectors/track, %d byte/sector\n",
             sides, tracks, sectors, secsize);
    free (buf);
    if (secsize != 512 || sectors != 9 || tracks != 80 || sides != 1) {
        exit (1);
    }
    buf = (unsigned char *) malloc (512 * sectors);
    for (i = 0; i < tracks; i++) {
        if (0 > lseek (fd, 512 * 18 * i, SEEK_SET)) {
            perror ("lseek");
            exit (1);
        }
        if (512 * 9 != read (fd, buf, 512 * 9)) {
            perror ("read");
            exit (1);
        }
        if (512 * 9 != fwrite (buf, 1, 512 * 9, f)) {
            perror ("fwrite");
            exit (1);
        }
        fprintf (stderr, ".");
    }
    fclose (f);
    fprintf (stderr, "\n");
    free (buf);
    return 0;
}
