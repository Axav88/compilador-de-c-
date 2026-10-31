#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Definición del estado global
log_status global_status;

static const char *level_to_string(message_level lvl) {
    switch (lvl) {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_ERROR:   return "ERROR";
        case LOG_LEVEL_FATAL:   return "FATAL";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        default:                return "UNKNOWN";
    }
}

void logger_init(logger_t *log, const char *environment, const char **messages, bool mode, const char *path) {
    if (!log) return;
    memset(log, 0, sizeof(logger_t));

    if (path) {
        strncpy(log->log_path, path, log_path_size - 1);
    } else {
        strncpy(log->log_path, "compiler_debug.log", log_path_size - 1);
    }

    // Configurar modo: true = global_status, false = local_status
    if (mode) {
        log->active_status = &global_status;
    } else {
        log->active_status = &log->local_status;
    }

    if (environment) {
        strncpy(log->active_status->environment, environment, logger_environment_size - 1);
    }
    log->active_status->status_messages = messages;
    log->active_status->status_flag = 0;
    log->active_status->file_code = 0;
    log->active_status->file_line = 1;

    timespec_get(&log->start_time, TIME_UTC);
}

void logger_init_default(logger_t *log, const char *path) {
    logger_init(log, "GLOBAL", NULL, true, path);
}

int logger_get_status(const logger_t *log) {
    return log && log->active_status ? log->active_status->status_flag : -1;
}

int logger_set_status(logger_t *log, uint8_t code, unsigned int line, int flag, const char *extra_msg) {
    if (!log || !log->active_status) return -1;

    log->active_status->file_code = code;
    log->active_status->file_line = line;
    log->active_status->status_flag = flag;

    if (extra_msg) {
        strncpy(log->active_status->status_message, extra_msg, logger_status_message_size - 1);
    } else if (log->active_status->status_messages && log->active_status->status_messages[flag]) {
        strncpy(log->active_status->status_message, log->active_status->status_messages[flag], logger_status_message_size - 1);
    }

    snprintf(log->active_status->full_message, logger_full_message_size,
             "[%s][F:%u:L:%u] Code %d: %s",
             log->active_status->environment,
             log->active_status->file_code,
             log->active_status->file_line,
             log->active_status->status_flag,
             log->active_status->status_message);

    return 0;
}

int logger_print_log(const logger_t *log, message_level level, const char *fmt, ...) {
    char user_buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(user_buffer, sizeof(user_buffer), fmt, args);
    va_end(args);

    const char *env = (log && log->active_status) ? log->active_status->environment : "APP";

    // Imprimir en consola con formato estructurado
    printf("[%s][%s] %s\n", env, level_to_string(level), user_buffer);

    // Persistir en el archivo de log indicado
    if (log && log->log_path[0] != '\0') {
        FILE *fp = fopen(log->log_path, "a");
        if (fp) {
            fprintf(fp, "[%s][%s] %s\n", env, level_to_string(level), user_buffer);
            fclose(fp);
        }
    }
    return 0;
}

void logger_print_status(const logger_t *log) {
    if (log && log->active_status) {
        printf("%s\n", log->active_status->full_message);
    }
}