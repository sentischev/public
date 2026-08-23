
#ifndef _GDBUS_H
#define _GDBUS_H

#include <glib.h>
#include <gio/gio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GDBUS_SPP_READ_BUF_SIZE			512

int gdbus_init();
int gdbus_run();

#ifdef __cplusplus
}
#endif

#endif
