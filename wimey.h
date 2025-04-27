/*
 * wimey.h - Wimey command line management library
 *
 * Copyright (C) 2025 Davide Usberti <usertibox@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef __WIMEY_H
#define __WIMEY_H

#ifdef _cplusplus
extern "C" {
#endif

#define WIMEY_OK 1
#define WIMEY_ERR 0

/* Debug level value:
 * 0 - Errors only
 * 1 - Errors & Warns
 * 2 - Errors, Warns and Info 
 */
#define LOG_ERR_ONLY 0
#define LOG_ERR_AND_WARNS 1
#define LOG_ALL 2

struct wimey_config_t {
	int log_level;
	char name[32]; /* program name */
	char *desctipion; /* program description */
	char *version; /* version: x.x.x */	
};

struct wimey_command_t {
	char *key; /* Command key */
	int has_value; /* Boolean example: run <value?> */
	int is_value_required;
	char *value_name; /* or NULL */
	void (*callback)(const char *value);
};

/* ------ Public API ------ */
int wimey_init(void);
int wimey_set_config(struct wimey_config_t *conf);
struct wimey_config_t wimey_get_config(void);
int wimey_add_command(struct wimey_command_t cmd);
struct __wimey_command_node *wimey_get_commands_head(void);
void wimey_free_all(void);

#ifdef _cplusplus
}
#endif

#endif /* __WIMEY_H */
