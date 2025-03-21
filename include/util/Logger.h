#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_ERR(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, "[ERR] %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)

#define LOG(fmt, ...) \
  do { if (DEBUG) fprintf(stdout, "[LOG] %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)

#define LOG_ERR_NOARGS(msg) \
  do { if (DEBUG) fprintf(stderr, "[ERR] %s:%d %s\n", __FILE__, __LINE__, msg); } while (0)

#define LOG_NOARGS(msg) \
  do { if (DEBUG) fprintf(stdout, "[LOG] %s:%d %s\n", __FILE__, __LINE__, msg); } while (0)

#endif
