// SPDX-License-Identifier: GPL-2.0
#ifndef FORTYTWO_H
#define FORTYTWO_H

#include <linux/mm.h>

#define LOGIN "yoson"
#define LOGIN_LEN (sizeof(LOGIN) - 1)
#define MAX_FOO_LEN PAGE_SIZE

extern const struct file_operations id_fops;
extern const struct file_operations jiffies_fops;
extern const struct file_operations foo_fops;

#endif
