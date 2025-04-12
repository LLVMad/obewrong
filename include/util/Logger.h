#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_ERR(fmt, ...)                                                      \
  do {                                                                         \
    if (DEBUG)                                                                 \
      fprintf(stderr, "[ERR] %s:%d " fmt, __FILE__, __LINE__, __VA_ARGS__);    \
  } while (0)

#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    if (DEBUG)                                                                 \
      fprintf(stdout, "[LOG] %s:%d " fmt, __FILE__, __LINE__, __VA_ARGS__);    \
  } while (0)

#define WARN(fmt, ...)                                                         \
  do {                                                                         \
    fprintf(stderr, "[WARNING] " fmt, __VA_ARGS__);                               \
  } while (0)

#define ERR(fmt, ...)                                                          \
  do {                                                                         \
    fprintf(stderr, "[ERROR] " fmt, __VA_ARGS__);                                 \
  } while (0)


#endif
