/*
 * wimey.c - Wimey command line management library
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

/**
 * TODO: (DONE) Commands recognition 
 * TODO: (DONE) Value parsers es. wimey_val_to_int()
 * TODO: Prefix config
 * TODO: "[key]=<value>" or "[key] <value>" config
 * TODO: Arguments management
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "wimey.h"

/* For the dependency-less philosophy, 
 * we do not use `stdbool.h` */
typedef int bool;
#define true 1
#define false 0

#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;31m"

struct __wimey_command_node {
	struct wimey_command_t cmd;
	struct __wimey_command_node *next;
};

static struct {
	struct __wimey_command_node *cmds_head;
} wimey_dict = {
	.cmds_head = NULL
};

struct wimey_config_t wimey_conf = {
	.log_level = LOG_ALL
};

#define ERR(msg, ...) \
	do { \
		fprintf(stderr, RED "ERROR " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

#define WARN(msg, ...) \
	do { \
		if(wimey_conf.log_level >= LOG_ERR_AND_WARNS) \
			printf(YELLOW "WARN  " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

#define INFO(msg, ...) \
	do { \
		if(wimey_conf.log_level >= LOG_ALL) \
			printf(GREEN "INFO  " RESET msg "\n", ##__VA_ARGS__); \
	} while(0)

/* ------- config functions ------- */

/* Set global lib configuration 
 * -----------------------------
 * Arguments:
 * 	struct wimey_config_t *conf */
int wimey_set_config(struct wimey_config_t *conf)
{
	if (conf == NULL)
		return WIMEY_ERR;

	wimey_conf = *conf;
	return WIMEY_OK;
}

/* Get global lib configuration
 * ---------------------------------
 *  Returns: struct wimey_config_t */
struct wimey_config_t wimey_get_config(void)
{
	return wimey_conf;
}

/* Internal helper to create a command node */
struct __wimey_command_node
*wimey_create_command_node(struct wimey_command_t new_cmd)
{
	struct __wimey_command_node *new_node =
	    (struct __wimey_command_node
	     *)(malloc(sizeof(struct __wimey_command_node)));

	if (!new_node) {
		ERR("Memory allocation failed during command node creation");
		return NULL;
	}

	new_node->cmd = new_cmd;
	new_node->next = NULL;
	return new_node;
}

/* This function pulls back a new command 
 * to the commands dynamic list */
int wimey_add_command(struct wimey_command_t cmd)
{
	struct __wimey_command_node *new_cmd = wimey_create_command_node(cmd);

	if (!new_cmd) {
		ERR("Failed to push back new command, invalid argument.");
		return WIMEY_ERR;
	}

	if (wimey_dict.cmds_head == NULL) {
		wimey_dict.cmds_head = new_cmd;
		return WIMEY_OK;
	}

	struct __wimey_command_node *current = wimey_dict.cmds_head;

	while (current->next != NULL) {
		current = current->next;
	}

	current->next = new_cmd;
	return WIMEY_OK;
}

/* Check if a string is a command */
bool __wimey_is_str_a_command(char *str)
{
	struct __wimey_command_node *current = wimey_get_commands_head();

	if (current == NULL)
		return false;

	while (current != NULL) {
		if (strncmp(current->cmd.key, str, strlen(current->cmd.key)) ==
		    0)
			return true;
		current = current->next;
	}

	return false;
}

/* This function returns the head of the commands list
 * or NULL if the list is empty  */
struct __wimey_command_node *wimey_get_commands_head(void)
{
	return wimey_dict.cmds_head;
}

/* Process a specific command with its value if needed */
bool __wimey_process_command(struct __wimey_command_node *cmd_node, char *value)
{
	if (cmd_node->cmd.has_value) {
		cmd_node->cmd.callback(value);
		return true;
	}

	cmd_node->cmd.callback(NULL);
	return true;
}

/* Internal function that iterates the
 * command line buffer and search commands.
 * Then executes callbacks and returns
 */
int __wimey_parse_commands_from_buff(int argc, char **argv)
{
	if (argc < 2) {
		ERR("Argc < 2, but there are commands in the dictionary");
		goto err;
	}

	for (int arg_i = 1; arg_i < argc; arg_i++) {
		struct __wimey_command_node *current =
		    wimey_get_commands_head();

		if (current == NULL) {
			ERR("No commands registered in dictionary");
			goto err;
		}

		while (current != NULL) {
			if (strcmp(current->cmd.key, argv[arg_i]) == 0) {
				INFO("Found command: %s", current->cmd.key);

				if (current->cmd.is_value_required
				    && arg_i + 1 >= argc) {
					ERR("Command %s requires value `%s` but none provided", 
							current->cmd.key, current->cmd.value_name);
					goto err;
				}

				if (current->cmd.has_value && arg_i + 1 < argc) {
					if (!__wimey_is_str_a_command
					    (argv[arg_i + 1])) {
						__wimey_process_command(current,
									argv
									[arg_i +
									 1]);
						arg_i++;
						return WIMEY_OK;
					}
				}

				__wimey_process_command(current, NULL);
				return WIMEY_OK;
			}
			current = current->next;
		}

		WARN("Unknown command or argument: %s", argv[arg_i]);
	}

err:
	ERR("Error found during command parsing, invalid input");
	return WIMEY_ERR;
}

/* --------------- Utiliy functions ---------------- */

/* This is a generic char* to long converter that 
 * can be used for different subtypes 
 * such us uint16_t int32_t etc..  */
int wimey_val_to_long(const char *val) {
	long res;
	char *p_end;

	if(val == NULL || !val)
		goto err;
	
	errno = 0;
	res = strtol(val, &p_end, 10);

	if(errno == ERANGE || *p_end != '\0')
		goto err;

	return res;

err:
	ERR("Failed to convert value, check your input: %s", val);
	return WIMEY_ERR;
}

/* Converter function from cmd/args value to int */
int wimey_val_to_int(const char *val) {
	return (int)wimey_val_to_long(val);	
}

/* Easy converter function from cmd/args value to float using `strtof()` */
float wimey_val_to_float(const char *val) {
	float res; 
	char *p_end;

	if(val == NULL || !val)
		goto err;

	res = strtof(val, &p_end);

	if(p_end == val)
		goto err;

	return res;

err:
	ERR("Failed to convert value, check your input: %s", val);
	return WIMEY_ERR;
}

/* Easy function to convert a command/args value 
 * to double using `strtod()` */
double wimey_val_to_double(const char *val) {
	double res;
	char *p_end;

	if(val == NULL || !val)
		goto err;

	res = strtod(val, &p_end);

	if(p_end == val)
		goto err;

	return res;

err:
	ERR("Failed to convert value, check your input: %s", val);
	return WIMEY_ERR;
}

/* Value converter form char* to unsigned long long  */
uint64_t wimey_val_to_u64(const char *val) {
	long res;
	char *p_end;

	if(val == NULL || !val)
		goto err;
	
	errno = 0;

	/* This function uses from stdlib.h the function
	 *  strtoll() instead of strtol() to convert strings into
	 *  uint64_t alias of unsigned long long */
	res = strtoll(val, &p_end, 10);

	if(errno == ERANGE || *p_end != '\0')
		goto err;

	return res;

err:
	ERR("Failed to convert value, check your input: %s", val);
	return WIMEY_ERR;
}

/* Basic casting of function `wimey_val_to_long` 
 * to return data as char */
char wimey_val_to_char(const char *val) {
	return (char)wimey_val_to_long(val);
}

/* Default library configuration and
 * internal functions caller. */
int wimey_init(void)
{
	wimey_dict.cmds_head = NULL;
	return WIMEY_OK;
}

/* This function free all the lists  */
void wimey_free_all(void)
{
	struct __wimey_command_node *current = wimey_dict.cmds_head;
	struct __wimey_command_node *tmp;

	while (current != NULL) {
		tmp = current->next;

		free(current);
		current = tmp;
	}

	wimey_dict.cmds_head = NULL;
}

#ifndef MAIN_TEST

void command_hello(const char *value)
{
	printf("Hello: %s\n", value ? value : "(no value)");
	char op = wimey_val_to_char(value);
	printf("Result * 2 = %d", op * 2);
}

void command_goodbye(const char *value)
{
	printf("Goodbye: %s\n", value ? value : "(no value)");
}

int main(int argc, char **argv)
{
	//ERR("This is an error %s", "Invalid input");
	//WARN("This is a warn %s", "don't ignore me");
	//INFO("This is an info message %s", "you can ignore me");

	if (wimey_init() != WIMEY_OK) {
		ERR("Failed to initialize Wimey");
		return 1;
	}

	struct wimey_command_t cmd1 = {
		.key = "hello",
		.has_value = true,
		.is_value_required = true,
		.value_name = "Name",
		.callback = command_hello
	};

	struct wimey_command_t cmd2 = {
		.key = "goodbye",
		.has_value = false,
		.is_value_required = false,
		.value_name = "Name",
		.callback = command_goodbye
	};

	if (wimey_add_command(cmd1) != WIMEY_OK) {
		ERR("Failed to add command: %s", cmd1.key);
	} else {
		INFO("Command added: %s", cmd1.key);
	}

	if (wimey_add_command(cmd2) != WIMEY_OK) {
		ERR("Failed to add command: %s", cmd2.key);
	} else {
		INFO("Command added: %s", cmd2.key);
	}

	struct __wimey_command_node *current = wimey_get_commands_head();

	while (current != NULL) {
		printf("- %s\n", current->cmd.key);
		current = current->next;
	}

	if (argc > 1) {
		__wimey_parse_commands_from_buff(argc, argv);
	}
	wimey_free_all();

	return 0;
}

#endif
