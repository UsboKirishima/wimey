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
 * TODO: Adding arguments functions
 * TODO: Change variable with arguments so get var as pointer in func adder
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

static struct {
	struct __wimey_command_node *cmds_head;
	struct __wimey_argument_node *args_head;
} wimey_dict = {
	.cmds_head = NULL,
	.args_head = NULL
};

struct wimey_config_t wimey_conf = {
	.log_level = LOG_ALL
};

#undef ERR
#undef WARN
#undef INFO

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
	
	if(!new_node) {
		ERR("Failed to allocate command.");
		return NULL;
	}

	/* heap struct initiaization */
	memset(new_node, 0, sizeof(*new_node));

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

static struct __wimey_command_node 
*__wimey_get_command_node(char *str) {
	struct __wimey_command_node *current = wimey_get_commands_head();

	if(current == NULL)
		return NULL;

	while(current != NULL) {
		bool is_valid_key = strcmp(str, current->cmd.key);

		if(is_valid_key == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

bool __wimey_check_command(char *str) {
	return __wimey_get_command_node(str) != NULL;
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
int __wimey_parse_commands(int argc, char **argv)
{
	if(wimey_get_commands_head() == NULL)
		return WIMEY_OK;
	
	if (argc < 2) {
		ERR("Argc < 2, but there are commands in the dictionary");
		goto err;
	}

	for (int arg_i = 1; arg_i < argc; arg_i++) {
		char *current_cmd = argv[arg_i];
		bool is_cmd_in_dict = __wimey_check_command(current_cmd);
		

		if(!is_cmd_in_dict) 
			continue;

		struct __wimey_command_node *node = __wimey_get_command_node(current_cmd);
		
		if(!node) {
			ERR("Failed to get node pointer");
			goto err;
		}

		if(node->cmd.is_value_required 
				&& arg_i + 1 >= argc) {
			ERR("Command %s requires value `%s` but none provided", 
					node->cmd.key, node->cmd.value_name);
			goto err;
		}

		if(node->cmd.has_value && arg_i + 1 < argc) {
			INFO("Found command: %s", node->cmd.key);

			if (node->cmd.is_value_required
					&& arg_i + 1 >= argc) {
				ERR("Command %s requires value `%s` but none provided", 
						node->cmd.key, node->cmd.value_name);
					goto err;
			}

			if (node->cmd.has_value && arg_i + 1 < argc) {
				if (!__wimey_is_str_a_command
						(argv[arg_i + 1])) {

					__wimey_process_command(node, argv[arg_i + 1]);
					arg_i++;
					return WIMEY_OK;
				}
			}

			__wimey_process_command(node, NULL);
			return WIMEY_OK;
		}
	}
err:		
	ERR("Error found during command parsing, invalid input");
	return WIMEY_ERR;
}

/* --------------- Arguments functions ------------- */

struct __wimey_argument_node
*wimey_create_argument_node(struct wimey_argument_t new_arg)
{
	struct __wimey_argument_node *new_node =
	    (struct __wimey_argument_node
	     *)(malloc(sizeof(struct __wimey_argument_node)));

	if (!new_node) {
		ERR("Allocation failed of argument");
		return NULL;
	}

	memset(new_node, 0, sizeof(*new_node));

	new_node->argument = new_arg;
	new_node->next = NULL;
	return new_node;
}

int wimey_add_argument(struct wimey_argument_t argument) {
	struct __wimey_argument_node *new_arg = wimey_create_argument_node(argument);

	if (!new_arg) {
		ERR("Failed to push back new argument, invalid argument.");
		return WIMEY_ERR;
	}

	if (wimey_dict.args_head == NULL) {
		wimey_dict.args_head = new_arg;
		return WIMEY_OK;
	}

	struct __wimey_argument_node *current = wimey_dict.args_head;

	while (current->next != NULL) {
		current = current->next;
	}

	current->next = new_arg;
	return WIMEY_OK;
}

struct __wimey_argument_node *wimey_get_arguments_head(void)
{
	return wimey_dict.args_head;
}

/* This function gets a string and returns if the string is
 * contained into the arguments list */
/*bool __wimey_check_argument(char *str) {
	struct __wimey_argument_node *current = wimey_get_arguments_head();

	if(current == NULL)
		return false;
	
	while(current != NULL) {
		bool is_valid_lkey = strcmp(str, current->argument.long_key);
		bool is_valid_skey = strcmp(str, current->argument.short_key);

		if(is_valid_lkey == 0 || is_valid_skey == 0) {
			INFO("Argument found: %s", str);
			return true;
		}
		
		current = current->next;
	}
	return true;
}*/

static struct __wimey_argument_node 
*__wimey_get_argument_node(char *str) {
	struct __wimey_argument_node *current = wimey_get_arguments_head();

	if(current == NULL)
		return NULL;

	while(current != NULL) {
		bool is_valid_lkey = strcmp(str, current->argument.long_key);
		bool is_valid_skey = strcmp(str, current->argument.short_key);

		if(is_valid_lkey == 0 || is_valid_skey == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/* Inline function to return true if the str is 
 * a recognized argument else returns false if 
 * the str is out of args list*/
static bool __wimey_check_argument(char *str) {
	return __wimey_get_argument_node(str) != NULL;
}

int __wimey_parse_arguments(int argc, char **argv) {
	if(wimey_get_arguments_head() == NULL)
		return WIMEY_OK;
	
	for(int arg_i = 0; arg_i < argc; arg_i++) {
		char *current_arg = argv[arg_i];
		bool is_arg_in_dict = __wimey_check_argument(current_arg);

		if(!is_arg_in_dict)
			continue;

		struct __wimey_argument_node *node = __wimey_get_argument_node(current_arg);

		if(node->argument.is_value_required
				&& arg_i + 1 >= argc) {
			ERR("Argument %s requires value `%s` but none provided", 
					node->argument.long_key, node->argument.value_name);
			goto err;
		}

		if(node->argument.has_value && arg_i + 1 < argc) {
			//todo
			return WIMEY_OK;
		}
	}

err:
	ERR("Error found during argument parsing, invalid input");
	return WIMEY_ERR;
}
/* --------------- Utiliy functions ---------------- */

/* This is a generic char* to long converter that 
 * can be used for different subtypes 
 * such us uint16_t int32_t etc..  */
long wimey_val_to_long(const char *val) {
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
	wimey_dict.args_head = NULL;
	return WIMEY_OK;
}

/* Wrapper to internal helper  */
int wimey_parse(int argc, char **argv) {
	int cmds_parse = __wimey_parse_commands(argc, argv);
	int args_parse = __wimey_parse_arguments(argc, argv);

	return cmds_parse && args_parse;
}

/* This function free all the lists  */
void wimey_free_all(void)
{
	struct __wimey_command_node *current_cmd = wimey_dict.cmds_head;
	struct __wimey_argument_node *current_arg = wimey_dict.args_head;
	
	struct __wimey_command_node *tmp_cmd;
	struct __wimey_argument_node *tmp_arg;
	
	/* Deallocates commands list */
	while (current_cmd != NULL) {
		tmp_cmd = current_cmd->next;

		free(current_cmd);
		current_cmd = tmp_cmd;
	}
	
	wimey_dict.cmds_head = NULL;
	
	/* Deallocates arguments list */
	while(current_arg != NULL) {
		tmp_arg = current_arg->next;

		free(current_arg);
		current_arg = tmp_arg;
	}

	wimey_dict.args_head = NULL;
}


