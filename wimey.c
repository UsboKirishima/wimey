/*
 * wimey.c - Wimey command line management library
 *
 * Copyright (C) 2025 Davide Usberti <usbertibox@gmail.com>
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
 *
 * =================== Informations About Library ========================
 * This is a simple library made to parse both commands and arguments,
 * It can be used to make a command line tool.
 *
 * So we need to understand what a command is:
 * 1. A command is a direction given from the user to perform something,
 *    in general when a command is specified it calls a function that do
 *    something. For example:
 *    	Here we have a command line tool called "npm" used to manage
 *    	node.js dependencies in a js project. If we look at the command: 
 *    	"npm install electron" we can individuate some expressions: 
 *    	"npm <cmd> <arg>", and the library simply interprets this as 
 *	void install(char *argument);
 *    So how can we manage all the commands? Simple, we add all the 
 *    commands in a linked list, then we get the command line 
 *    arguments "argc" and "argv" and parse it by iterating on our commands
 *    list, when we find a command, we check if the command requires a value,
 *    in case of positive result we get the value and we sent it to a callback
 *    function that we run.
 *
 * 2. An argument is like a constant passed by user using command line,
 *    in general arguments are used to specify settings, manage output,
 *    passing informations such as files, addresses, ports etc...
 *    For example: "ffmpeg --version" displays the version of ffmpeg,
 *    in this case the argument "--version" does not require a value.
 *    But if we look at "pacman -S vim" we can see that we have an argument
 *    called "-S" in short version or "--sync" in long version that requires
 *    the packages name as value. So we can treat it as a variable 
 *    "char *sync = NULL;" and when the user passes "--sync" as parameter
 *    the program dereferences the "sync" variable passed as pointer to a function
 *    and modify it adding the user passed value.
 *
 * =================== Project Design & Todos  ========================
 *
 * TODO: "[key]=<value>" or "[key] <value>" configuration format
 *
 * TODO: (DONE) Generate help
 * TODO: (DONE) Command recognition 
 * TODO: (DONE) Value parsers e.g. wimey_val_to_int()
 * TODO: (DONE) Argument adding functions
 * TODO: (DONE) Change variable handling with arguments to get var as pointer in adder function
 * TODO: (DONE) Argument management
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "wimey.h"

/* For dependency-free design, 
 * we avoid using `stdbool.h` */
typedef int bool;
#define true 1
#define false 0

/* Errors are handled in all functions by two defines
 * WIMEY_OK = 1 (When a function returns successully) 
 * WIMEY_ERR = 0 (When a fucntion returns failure) 
 * ...see header file
 */

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

struct wimey_argument_t help_arg = {
		.long_key = "--help",
		.short_key = "-h",
		.has_value = false,
		.is_value_required = false,
		.value_dest = NULL,
		.value_name = NULL,
		.value_type = WIMEY_BOOL,
		.desc = "Show help list"
};

#undef ERR
#undef WARN
#undef INFO

/* These debug macros are different from the
 * macros declared in wimey.h because here we need
 * to follow the debug level set with the lib 
 * configuration struct  */

#define ERR(msg, ...) \
    do { \
	fprintf(stderr, RED "ERROR " RESET msg "\n", ##__VA_ARGS__); \
    } while (0)

#define WARN(msg, ...) \
    do { \
	if (wimey_conf.log_level >= LOG_ERR_AND_WARNS) \
	    printf(YELLOW "WARN  " RESET msg "\n", ##__VA_ARGS__); \
    } while (0)

#define INFO(msg, ...) \
    do { \
	if (wimey_conf.log_level >= LOG_ALL) \
	    printf(GREEN "INFO  " RESET msg "\n", ##__VA_ARGS__); \
    } while (0)


/* prototypes */
void __wimey_print_help(int argc, char **argv);

/* ------- Configuration functions ------- */

/* Set global library configuration 
 * --------------------------------
 * Arguments:
 *  struct wimey_config_t *conf - Pointer to configuration structure */
int wimey_set_config(struct wimey_config_t *conf) {
	if (conf == NULL)
		return WIMEY_ERR;

	wimey_conf = *conf;
	return WIMEY_OK;
}

/* Get global library configuration
 * -------------------------------
 * Returns: struct wimey_config_t - Current configuration */
struct wimey_config_t wimey_get_config(void) {
	return wimey_conf;
}

/* Internal helper to create a command node */
struct __wimey_command_node
*wimey_create_command_node(struct wimey_command_t new_cmd) {
	struct __wimey_command_node *new_node =
	    (struct __wimey_command_node
	     *)(malloc(sizeof(struct __wimey_command_node)));

	if (!new_node) {
		ERR("Failed to allocate command.");
		return NULL;
	}

	/* Initialize heap structure */
	memset(new_node, 0, sizeof(*new_node));

	new_node->cmd = new_cmd;
	new_node->next = NULL;
	return new_node;
}

/* Adds a new command to the commands linked list */
int wimey_add_command(struct wimey_command_t cmd) {
	struct __wimey_command_node *new_cmd = wimey_create_command_node(cmd);

	if (!new_cmd) {
		ERR("Failed to append new command, invalid argument.");
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

/* Check if a string matches a registered command */
bool __wimey_is_str_a_command(char *str) {
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

/* Returns the head of the commands list
 * or NULL if the list is empty */
struct __wimey_command_node *wimey_get_commands_head(void) {
	return wimey_dict.cmds_head;
}

/* Given a string returns the command node */
static struct __wimey_command_node
*__wimey_get_command_node(char *str) {
	struct __wimey_command_node *current = wimey_get_commands_head();

	if (current == NULL)
		return NULL;

	while (current != NULL) {
		bool is_valid_key = strcmp(str, current->cmd.key);

		if (is_valid_key == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/* Given a string returns if it's a valid command  */
bool __wimey_check_command(char *str) {
	return __wimey_get_command_node(str) != NULL;
}

/* Process a specific command with its value if needed */
bool __wimey_process_command(struct __wimey_command_node *cmd_node, char *value) {
	if (cmd_node->cmd.has_value) {
		cmd_node->cmd.callback(value);
		return true;
	}

	cmd_node->cmd.callback(NULL);
	return true;
}

/* Internal function that iterates through
 * command line arguments searching for commands.
 * Executes callbacks and returns status */
int __wimey_parse_commands(int argc, char **argv) {
	if (wimey_get_commands_head() == NULL)
		return WIMEY_OK;

	if (argc < 2) {
		ERR("Argc < 2, but commands exist in the dictionary");
		goto err;
	}

	for (int arg_i = 0; arg_i < argc; arg_i++) {
		char *current_cmd = argv[arg_i];
		bool is_cmd_in_dict = __wimey_check_command(current_cmd);
		bool overflow = arg_i + 1 >= argc;

		if (!is_cmd_in_dict)
			continue;

		struct __wimey_command_node *node =
		    __wimey_get_command_node(current_cmd);

		if (!node) {
			ERR("Failed to get node pointer");
			goto err;
		}

		if (node->cmd.is_value_required && overflow) {
			ERR("Command %s requires value `%s` but none provided",
			    node->cmd.key, node->cmd.value_name);
			goto err;
		}

		if (node->cmd.has_value && !overflow) {
			INFO("Found command: %s", node->cmd.key);

			if (node->cmd.is_value_required && overflow) {
				ERR("Command %s requires value `%s` but none provided", node->cmd.key, node->cmd.value_name);
				goto err;
			}

			if (node->cmd.has_value && !overflow) {
				if (!__wimey_is_str_a_command(argv[arg_i + 1])) {

					__wimey_process_command(node,
								argv[arg_i +
								     1]);
					arg_i++;
					//    return WIMEY_OK;
					continue;
				}
			}

			__wimey_process_command(node, NULL);
			//return WIMEY_OK;
			continue;
		}
	}

	return WIMEY_OK;
err:
	ERR("Error during command parsing, invalid input");
	return WIMEY_ERR;
}

/* --------------- Argument functions ------------- */

/* This function get as parameter wimey_argument_t
 * and put it into a new allocated node, essentialy
 * a "next" pointer and our value (wimey_argument_t) */
struct __wimey_argument_node
*wimey_create_argument_node(struct wimey_argument_t new_arg) {
	struct __wimey_argument_node *new_node =
	    (struct __wimey_argument_node
	     *)(malloc(sizeof(struct __wimey_argument_node)));

	if (!new_node) {
		ERR("Argument allocation failed");
		return NULL;
	}

	memset(new_node, 0, sizeof(*new_node));

	new_node->argument = new_arg;
	new_node->next = NULL;
	return new_node;
}

/* Add argument to arguments dynamic list */
int wimey_add_argument(struct wimey_argument_t argument) {

	/* If argument has no value or value isn't required
	 * we set type as bool. For example --help argument
	 * doesn't need values and can be treated as boolean.
	 *
	 * Future enhancement: handle arguments that can be
	 * either boolean or take a value. Example:
	 *  Boolean case:
	 *      Command: program --help
	 *      Output: Type --help <category>
	 *  
	 *  Value case:
	 *      Command: program --help banana
	 *      Output: Help for Banana
	 *          ...
	 */
	if (!argument.is_value_required || !argument.has_value) {
		argument.value_type = WIMEY_BOOL;
		argument.is_value_required = true;
		argument.has_value = true;
	}

	struct __wimey_argument_node *new_arg =
	    wimey_create_argument_node(argument);

	if (!new_arg) {
		ERR("Failed to append new argument, invalid argument.");
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

/* Internal function to get the arguments head node 
 * but also avaible as public out of the library to iterate
 * the list of args  */
struct __wimey_argument_node *wimey_get_arguments_head(void) {
	return wimey_dict.args_head;
}

/* Get the node of a specific argument given by string */
static struct __wimey_argument_node
*__wimey_get_argument_node(char *str) {
	struct __wimey_argument_node *current = wimey_get_arguments_head();

	if (current == NULL)
		return NULL;

	while (current != NULL) {
		bool is_valid_lkey = strcmp(str, current->argument.long_key);
		bool is_valid_skey = strcmp(str, current->argument.short_key);

		if (is_valid_lkey == 0 || is_valid_skey == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/* Returns true if string is a registered argument,
 * false if not found in arguments list */
static bool __wimey_check_argument(char *str) {
	return __wimey_get_argument_node(str) != NULL;
}

/* Internal function to parse all the arguments,
 * this function also do checks and deref of
 * pointers shared in the arguments  */
int __wimey_parse_arguments(int argc, char **argv) {
	if (wimey_get_arguments_head() == NULL)
		return WIMEY_OK;

	for (int arg_i = 0; arg_i < argc; arg_i++) {
		char *current_arg = argv[arg_i];
		bool is_arg_in_dict = __wimey_check_argument(current_arg);
		bool overflow = arg_i + 1 > argc;

		if (!is_arg_in_dict)
			continue;

		struct __wimey_argument_node *node =
		    __wimey_get_argument_node(current_arg);

		if (node->argument.is_value_required 
				&& node->argument.value_type != WIMEY_BOOL 
				&& overflow) {
			
			ERR("Argument %s requires value `%s` but none provided",
			    node->argument.long_key, node->argument.value_name);
			goto err;
		}
		
		if (node->argument.has_value && !overflow) {

			if(strcmp(node->argument.long_key, help_arg.long_key) == 0) {
				__wimey_print_help(argc, argv);
				exit(EXIT_SUCCESS);

				return WIMEY_OK; /* Program will never reach this code  */
			}
			char *val = argv[arg_i + 1];
			
			/* Here we check the type of the argument */
			switch (node->argument.value_type) {
			case WIMEY_LONG:
				long res_l = wimey_val_to_long(val);
				*(long *)node->argument.value_dest = res_l;
				break;
			case WIMEY_DOUBLE:
				double res_d = wimey_val_to_double(val);
				*(double *)node->argument.value_dest = res_d;
				break;
			case WIMEY_STR:
				*(char **)node->argument.value_dest =
				    strdup(val);
				break;
			case WIMEY_BOOL:
				*(int *)node->argument.value_dest = true;
				break;
			default:
				ERR("Failed to resolve argument type");
				goto err;
			}
			continue;
		}
	}

	return WIMEY_OK;

err:
	ERR("Error during argument parsing, invalid input");
	return WIMEY_ERR;
}

/* --------------- Utility functions ---------------- */

/* This function simply create a privilaged node
 * that contains the help argument  */
int wimey_generate_help() {
	if(wimey_add_argument(help_arg) != WIMEY_OK) {
		ERR("Error during `--help` generation");
		return WIMEY_ERR;
	}
	
	return WIMEY_OK;
}

/* Internal helper to print the help
 * list and the program informations  */
void __wimey_print_help(int argc, char **argv) {
	(void)argc;

	struct __wimey_argument_node *arg = wimey_get_arguments_head();
	struct __wimey_command_node *cmd = wimey_get_commands_head();
	
	if(wimey_conf.name[0] != '\0' && wimey_conf.version != NULL)
		printf("%s (v%s)", wimey_conf.name, wimey_conf.version);
	
	if(wimey_conf.usage == NULL) {
		printf("\n%s [options] [arguments]", argv[0]);
	} else {
		printf("\nUsage: %s\n", wimey_conf.usage);
	}
	
	if(wimey_conf.description != NULL)
		printf("\n%s\n", wimey_conf.description);
	
	/* We need to take the max command key len */
	int max_cmd_len = 0;
    	while (cmd != NULL) {
        	int len = strlen(cmd->cmd.key);
        	if (len > max_cmd_len) max_cmd_len = len;
		cmd = cmd->next;
    	}

	int max_arg_len = 0;
	while(arg != NULL) {
		int alen = strlen(arg->argument.long_key) /* Adding len for --help  */
			+ strlen(arg->argument.short_key)  /* Adding len for -h */
			+ 2; /* Adding len for the 2 spaces between --help and -h */
		if(alen > max_arg_len) max_arg_len = alen;
		arg = arg->next;
	}
	
	int max_len = max_cmd_len > max_arg_len ? max_cmd_len : max_arg_len;

	cmd = wimey_get_commands_head();
	arg = wimey_get_arguments_head();

	printf("\n%s\n", "Commands:");
	while(cmd != NULL) {
		printf("  %-*s  %s\n", max_len, cmd->cmd.key, cmd->cmd.desc);
		cmd = cmd->next;
	}

	printf("\n%s\n", "Arguments: ");
	while(arg != NULL) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%s  %s", arg->argument.short_key, arg->argument.long_key);	
		printf("  %-*s  %s\n", max_len, buffer, arg->argument.desc);
		arg = arg->next;
	}
	
	if(wimey_conf.copyright != NULL)
		printf("\n%s\n", wimey_conf.copyright);

	if(wimey_conf.license != NULL)
		printf("This software is under %s license.", wimey_conf.license);
}

/* Generic string to long converter that can be used
 * for different integer subtypes (uint16_t, int32_t etc) */
long wimey_val_to_long(const char *val) {
	long res;
	char *p_end;

	if (val == NULL || !val)
		goto err;

	errno = 0;
	res = strtol(val, &p_end, 10);

	if (errno == ERANGE || *p_end != '\0')
		goto err;

	return res;

err:
	ERR("Conversion failed, check input: %s", val);
	return WIMEY_ERR;
}

/* Convert command/argument value to int */
int wimey_val_to_int(const char *val) {
	return (int)wimey_val_to_long(val);
}

/* Convert command/argument value to float using strtof() */
float wimey_val_to_float(const char *val) {
	float res;
	char *p_end;

	if (val == NULL || !val)
		goto err;

	res = strtof(val, &p_end);

	if (p_end == val)
		goto err;

	return res;

err:
	ERR("Conversion failed, check input: %s", val);
	return WIMEY_ERR;
}

/* Convert command/argument value to double using strtod() */
double wimey_val_to_double(const char *val) {
	double res;
	char *p_end;

	if (val == NULL || !val)
		goto err;

	res = strtod(val, &p_end);

	if (p_end == val)
		goto err;

	return res;

err:
	ERR("Conversion failed, check input: %s", val);
	return WIMEY_ERR;
}

/* Convert string to unsigned long long */
uint64_t wimey_val_to_u64(const char *val) {
	long res;
	char *p_end;

	if (val == NULL || !val)
		goto err;

	errno = 0;

	/* Uses strtoll() from stdlib.h to convert strings
	 * to uint64_t (unsigned long long) */
	res = strtoll(val, &p_end, 10);

	if (errno == ERANGE || *p_end != '\0')
		goto err;

	return res;

err:
	ERR("Conversion failed, check input: %s", val);
	return WIMEY_ERR;
}

/* Basic cast of wimey_val_to_long to return char */
char wimey_val_to_char(const char *val) {
	return (char)wimey_val_to_long(val);
}

/* Initialize library with default configuration */
int wimey_init(void) {
	wimey_dict.cmds_head = NULL;
	wimey_dict.args_head = NULL;
	return WIMEY_OK;
}

/* Wrapper for internal parsing functions */
int wimey_parse(int argc, char **argv) {
	int cmds_parse = __wimey_parse_commands(argc, argv);
	int args_parse = __wimey_parse_arguments(argc, argv);

	return cmds_parse && args_parse;
}

/* Free all allocated memory for commands and arguments */
void wimey_free_all(void) {
	struct __wimey_command_node *current_cmd = wimey_dict.cmds_head;
	struct __wimey_argument_node *current_arg = wimey_dict.args_head;

	struct __wimey_command_node *tmp_cmd;
	struct __wimey_argument_node *tmp_arg;

	/* Free commands list */
	while (current_cmd != NULL) {
		tmp_cmd = current_cmd->next;

		free(current_cmd);
		current_cmd = tmp_cmd;
	}

	wimey_dict.cmds_head = NULL;

	/* Free arguments list */
	while (current_arg != NULL) {
		tmp_arg = current_arg->next;

		free(current_arg);
		current_arg = tmp_arg;
	}

	wimey_dict.args_head = NULL;
}
