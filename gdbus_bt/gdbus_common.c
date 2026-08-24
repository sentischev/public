
#include <stdio.h>
#include <stdarg.h>

#include "gdbus_common.h"

#define TAG						"[GDBUS_COMMON]"

#if defined(_WINDOWS)
	#include <windows.h>

	#define WIN_COLOR_RED		(FOREGROUND_RED | FOREGROUND_INTENSITY)
	#define WIN_COLOR_YELLOW	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
	#define WIN_COLOR_GREEN		(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
	#define WIN_COLOR_GRAY		(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
	#define WIN_COLOR_WHITE		(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#else
	#define ANSI_COLOR_RED		"\x1b[31m"
	#define ANSI_COLOR_YELLOW	"\x1b[33m"
	#define ANSI_COLOR_GREEN	"\x1b[32m"
	#define ANSI_COLOR_GRAY		"\x1b[37m" /* almost white */
	#define ANSI_COLOR_WHITE	"\x1b[97m" /* hi-intensity white */
	#define ANSI_COLOR_RESET	"\x1b[0m"
#endif

gdbus_log_level_t gdbus_log_verbosity = GDBUS_LOG_LEVEL_INFO;

void gdbus_print(gdbus_log_level_t msg_level, const char *tag, const char *format, ...) {
	if (msg_level > gdbus_log_verbosity || gdbus_log_verbosity == GDBUS_LOG_LEVEL_NONE) {
		return;
	}

	va_list args;
	va_start(args, format);

#if defined(_WINDOWS)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD saved_attributes;
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	saved_attributes = consoleInfo.wAttributes;

	switch (msg_level) {
	case GDBUS_LOG_LEVEL_ERROR:
		SetConsoleTextAttribute(hConsole, WIN_COLOR_RED);
		break;
	case GDBUS_LOG_LEVEL_WARN:
		SetConsoleTextAttribute(hConsole, WIN_COLOR_YELLOW);
		break;
	default:
		SetConsoleTextAttribute(hConsole, WIN_COLOR_GREEN);
	}

	printf("%s ", tag);
	vprintf(format, args);

	SetConsoleTextAttribute(hConsole, saved_attributes);
#else
	switch (msg_level) {
	case GDBUS_LOG_LEVEL_ERROR:
		printf(ANSI_COLOR_RED);
		break;
	case GDBUS_LOG_LEVEL_WARN:
		printf(ANSI_COLOR_YELLOW);
		break;
	default:
		printf(ANSI_COLOR_GREEN);
	}

	printf("%s ", tag);
	vprintf(format, args);

	printf(ANSI_COLOR_RESET);
#endif

	va_end(args);
}
