#pragma once

#include "sqlite3/sqlite3.h"

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

int sqlite3_memvfs_init(sqlite3 *db,
                        char **pzErrMsg,
                        const sqlite3_api_routines *pApi);

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
