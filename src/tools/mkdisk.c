/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * $File$ - disk file builder
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
#include <sys/time.h>
#include <unistd.h>

/**
 ** Don't forget:
 ** (sectors * sides * tracks) / clsize < 4080 (12bit FAT)!!
 ** sectors < 256 (128?!)
 ** tracks < 256 (128?!)
 ** 1 <= sides <= 2
 ** secsize = 512 (always!)
 ** clsize < ?
 **/

unsigned        sides = 2;
unsigned        tracks = 80;
unsigned        sectors = 9;//50;
unsigned        secsize = 512;
unsigned        clsize = 2;

int             main (argc, argv)
    int             argc;
    char          **argv;
{
    FILE           *f;
    struct timeval  tp;
    if (argc != 2) {
        fprintf (stderr, "Usage: %s <outfile>\n", argv[0]);
        exit (1);
    }
    if (NULL == (f = fopen (argv[1], "wb"))) {
        perror ("open");
        exit (1);
    }
    printf ("Making disk: %d sides, %d tracks, %d sectors/track.\n",
            sides, tracks, sectors);
    printf ("             Sector size %d, %d sectors/cluster.\n",
            secsize, clsize);
    printf ("             Total diskspace is %d kB.\n",
            (secsize * sides * tracks * sectors) / 1024);
    fseek (f, (secsize * sides * tracks * sectors) - 1, 0);
    fputc (0, f);
    fflush (f);
    fseek (f, 0, 0);
    fputc (0x60, f);
    fputc (0x38, f);
    fseek (f, 8, 0);
    (void) gettimeofday (&tp, NULL);
    srand (tp.tv_usec);
    fputc (rand (), f);
    fputc (rand (), f);
    fputc (rand (), f);
    fputc (secsize % 256, f);
    fputc (secsize / 256, f);
    fputc (clsize, f);
    fputc (0x01, f);            /* reservierte Sektoren */
    fputc (0x00, f);
    fputc (0x02, f);            /* nfats */
    fputc (0xe0, f);            /* ndirs */
    fputc (0x00, f);
    fputc ((sides * tracks * sectors) % 256, f);        /* nsects */
    fputc ((sides * tracks * sectors) / 256, f);
    fputc (0xff, f);            /* media */
    fputc (0x0c, f);            /* fatsz */
    fputc (0x00, f);
    fputc (sectors, f);
    fputc (0x00, f);
    fputc (sides, f);
    fputc (0x00, f);
    fputc (0x00, f);            /* hidden sects */
    fputc (0x00, f);
    fputc (0x00, f);            /* execflag */
    fputc (0x00, f);
    fseek (f, 0x200, 0);        /* FAT 1 */
    fputc (0xf9, f);
    fputc (0xff, f);
    fputc (0xff, f);
    fseek (f, 0x1a00, 0);               /* FAT 2 */
    fputc (0xf9, f);
    fputc (0xff, f);
    fputc (0xff, f);

    fclose (f);
    return 0;
}
