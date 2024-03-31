/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - command line parser & system init
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  0.02.00 JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  09.06.2002  0.02.00 JH  Renamed io.c to st.c again (io.h conflicts with system headers)
*  19.06.2002  0.02.00 JH  -d option discontinued.
*/
static char     sccsid[] = "$Id: init.c,v 1.1 2004/04/22 17:17:04 cvs Exp $";
#include "config.h"
#include "st.h"
#include "68000.h"

int      Init()
{
    cpup = &cpu;
    if (!MemInit())
        return 0;
    if (disk[0].sides==0) FDCInit(0);
    if (disk[1].sides==0) FDCInit(1);
    IOInit();
    HWReset();
	IkbdReset();
    return 1;
}

