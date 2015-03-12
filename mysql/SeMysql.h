#ifndef __SE_MYSQL_H__
#define __SE_MYSQL_H__

#if defined(__linux)

#include <sys/socket.h>
#include <mysql/mysql.h>

#elif (defined(_WIN32) || defined(WIN32))

#include <winsock2.h>
#include "mysql.h"

#endif


#endif



