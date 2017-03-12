
#ifndef __SYSTEM_H
#define __SYSTEM_H

#define SYSC_EXIT (0)
#define SYSC_PUTI (1)
#define SYSC_NEW_THREAD (2)
#define SYSC_SLEEP (3)
#define SYSC_IDLE (4)
#define SYSC_GETCHAR (5)
#define SYSC_FORK (5)

/**********************************************************
** appel du systeme
***********************************************************/

PSW systeme(PSW m);

#endif
