
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "gdbus_bt.h"
#include "gdbus_wrapper.h"
#include "gdbus_common.h"

#define TAG						"[MAIN]"

gdbus_adapter_info_t adapters[3];

void print_help(const char *name) {
	printf("Usage:\n");
	printf("\t%s\n", name);
}

int main(int argc, char *argv[]) {
	int i;

	if (argc > 1) {
		print_help(argv[0]);
		return -1;
	}

	printf("%s starting ...\n", argv[0]);

	gdbus_init();

	int adapters_count = gdbus_get_adapters(adapters, GDBUS_ARRAY_SIZE(adapters));
	for (i = 0; i < adapters_count; ++i) {
		LOGI(TAG, "%s\n\tMAC: %s\n\tName: %s\n\tAlias: %s\n",
				adapters[i].path,
				adapters[i].address,
				adapters[i].name,
				adapters[i].alias);
	}

	gdbus_run();

	gdbus_release();

	return 0;
}
