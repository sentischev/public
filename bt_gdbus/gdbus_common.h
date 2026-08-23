
#ifndef _GDBUS_COMMON_H
#define _GDBUS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GDBUS_LOG_LEVEL_NONE,
	GDBUS_LOG_LEVEL_ERROR,
	GDBUS_LOG_LEVEL_WARN,
	GDBUS_LOG_LEVEL_INFO
} gdbus_log_level_t;

extern gdbus_log_level_t gdbus_log_verbosity;

void gdbus_print(gdbus_log_level_t level, const char *tag, const char *format, ...);

#define LOGI(tag, format, ...) gdbus_print(GDBUS_LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) gdbus_print(GDBUS_LOG_LEVEL_WARN, tag, format, ##__VA_ARGS__)
#define LOGE(tag, format, ...) gdbus_print(GDBUS_LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)

#define GDBUS_UNUSED(x)	(void)x

#ifdef __cplusplus
}
#endif

#endif
