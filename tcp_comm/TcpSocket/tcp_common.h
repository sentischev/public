
#ifndef _TCP_COMMON_H
#define _TCP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TCP_LOG_LEVEL_NONE,
	TCP_LOG_LEVEL_ERROR,
	TCP_LOG_LEVEL_WARN,
	TCP_LOG_LEVEL_INFO
} tcp_log_level_t;

extern tcp_log_level_t tcp_log_verbosity;

void tcp_common_print(tcp_log_level_t level, const char *tag, const char *format, ...);

#define LOGI(tag, format, ...) tcp_common_print(TCP_LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) tcp_common_print(TCP_LOG_LEVEL_WARN, tag, format, ##__VA_ARGS__)
#define LOGE(tag, format, ...) tcp_common_print(TCP_LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)

#define TCP_UNUSED(x)	((void)x)
#define TCP_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#ifdef __cplusplus
}
#endif

#endif
