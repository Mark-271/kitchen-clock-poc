/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef TOOLS_TOOLS_H
#define TOOLS_TOOLS_H

#include <time.h>

void inplace_reverse(char *str);
int get_yday(int mon, int day, int year);
void time2str(struct tm *tm, char *s);
void date2str(struct tm *tm, char *s);

#endif /* TOOLS_TOOLS_H */
