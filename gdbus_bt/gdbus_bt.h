
#ifndef _GDBUS_BT_H
#define _GDBUS_BT_H

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GDBUS_SPP_READ_BUF_SIZE			512

typedef struct {
	char path[16];		/* /org/bluez/hci0 */
	char address[18];	/* 12:34:56:AB:CD:EF */
	char name[16];
	char alias[16];
	bool powered;
	bool discoverable;
} gdbus_adapter_info_t;

int gdbus_get_adapters(gdbus_adapter_info_t *adapters, int adapters_array_size);

int gdbus_init();
int gdbus_run();
int gdbus_release();

#ifdef __cplusplus
}
#endif

#endif
