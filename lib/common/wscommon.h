#ifndef _WSCOMMON_H
#define _WSCOMMON_H

#define WSOK 1
#define WSNOK 0

void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
);

#endif
