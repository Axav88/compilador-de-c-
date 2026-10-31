#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Tamaños definidos según la diapositiva Logger III
#define logger_environment_size 10
#define logger_status_message_size 100
#define logger_full_message_size (20 + logger_environment_size + logger_status_message_size)
#define log_path_size 256

// Niveles de mensaje según Logger II
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING
} message_level;

// Estructura de estado según Logger IV
typedef struct {
    char environment[logger_environment_size];
    char full_message[logger_full_message_size];
    char status_message[logger_status_message_size];
    const char **status_messages;
    int status_flag;
    int status_shift;
    uint8_t file_code;
    unsigned int file_line;
} log_status;

// Estructura que emula la clase logger (Logger V y VI)
typedef struct {
    char log_path[log_path_size];
    log_status *active_status;
    log_status local_status;
    struct timespec start_time;
} logger_t;

// Variable global de estado (Logger V)
extern log_status global_status;

// Funciones del Logger (Logger VI)
void logger_init(logger_t *log, const char *environment, const char **messages, bool mode, const char *path);
void logger_init_default(logger_t *log, const char *path);
int logger_get_status(const logger_t *log);
int logger_print_log(const logger_t *log, message_level level, const char *fmt, ...);
int logger_set_status(logger_t *log, uint8_t code, unsigned int line, int flag, const char *extra_msg);
void logger_print_status(const logger_t *log);

#endif // LOGGER_H