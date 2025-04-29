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

#include <stdint.h>

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

#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"

#define ERR(msg, ...) \
	do { \
		fprintf(stderr, RED "ERROR " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

#define WARN(msg, ...) \
	do { \
		printf(YELLOW "WARN  " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

#define INFO(msg, ...) \
	do { \
		printf(GREEN "INFO  " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

struct wimey_config_t {
	int log_level;
	char name[32];	/* program name */
	char *description;	/* program description */
	char *version;	/* version: x.x.x */
};

struct wimey_command_t {
	char *key;	/* Command key */
	int has_value;	/* Boolean example: run <value?> */
	int is_value_required;
	char *value_name;	/* or NULL */
	char *desc;
	void (*callback)(const char *value);
};

struct wimey_argument_t {
	char *long_key; /* Example: --help */
	char *short_key; /* Example: -h */
	int has_value; /* Exampel: --port <value>  */
	int is_value_required; /* Example: --delay <delay> | --delay (default: 0) */
	char *value_name;
	char *desc;
	/* here no callback because arguments just 
	 * assign a value to a variable */
};

struct __wimey_command_node {
	struct wimey_command_t cmd;
	struct __wimey_command_node *next;
};

struct __wimey_argument_node {
	struct wimey_argument_t argument;
	struct __wimey_argument_node *next;
};

/* ------ Public API ------ */

/* Configuration & Init */
int wimey_init(void);
int wimey_set_config(struct wimey_config_t *conf);
struct wimey_config_t wimey_get_config(void);
void wimey_free_all(void);

/* This function is an universal wrapper 
 * both for commands and arguments  */
int wimey_parse(int argc, char **argv);

/* Commands */
int wimey_add_command(struct wimey_command_t cmd);

/* In some case if we need to iterate on the commands list
 * this functions return his head, so It's an internal 
 * function but public. */
struct __wimey_command_node *wimey_get_commands_head(void);

/* Utility function */
long wimey_val_to_long(const char *val);
int wimey_val_to_int(const char *val);
float wimey_val_to_float(const char *val);
double wimey_val_to_double(const char *val);
uint64_t wimey_val_to_u64(const char *val);
char wimey_val_to_char(const char *val);

#ifdef _cplusplus
}
#endif
#endif /* __WIMEY_H */
#pragma once
