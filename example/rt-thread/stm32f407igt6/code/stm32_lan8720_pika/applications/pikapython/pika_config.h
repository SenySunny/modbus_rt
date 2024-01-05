/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-24     SenyPC       the first version
 */
#ifndef APPLICATIONS_PIKAPYTHON_PIKA_CONFIG_H_
#define APPLICATIONS_PIKAPYTHON_PIKA_CONFIG_H_

#include "posix/string.h"

#define __linux__
#define PIKA_LINUX_COMPATIBLE       1


int gethostname(char *name, size_t len);
char * realpath(const char *file_name, char *resolved_name);

#endif /* APPLICATIONS_PIKAPYTHON_PIKA_CONFIG_H_ */
