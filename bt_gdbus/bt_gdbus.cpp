
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bt_gdbus.h"
#include "gdbus_wrapper.h"
#include "gdbus.h"

void print_help(const char *name) {
	printf("Usage:\n");
	printf("\t%s\n", name);
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		print_help(argv[0]);
		return -1;
	}

	printf("%s starting ...\n", argv[0]);

	gdbus_init();
	gdbus_run();

	return 0;
}
