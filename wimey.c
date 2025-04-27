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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
int wimey_set_config(struct wimey_config_t *conf) {
	if(conf == NULL)
		return WIMEY_ERR;
	
	wimey_conf = *conf;
	return WIMEY_OK;
} 

/* Get global lib configuration
 * ---------------------------------
 *  Returns: struct wimey_config_t */
struct wimey_config_t wimey_get_config(void) {
	return wimey_conf;
}

/* Internal helper to create a command node */
struct __wimey_command_node 
*wimey_create_command_node(struct wimey_command_t new_cmd) {
	struct __wimey_command_node *new_node = 
		(struct __wimey_command_node *)(malloc(sizeof(struct __wimey_command_node)));

	if(!new_node) {
		ERR("Memory allocation failed during command node creation");
		return NULL;
	}

	new_node->cmd = new_cmd;
	new_node->next = NULL;
	return new_node;
}

/* This function pulls back a new command 
 * to the commands dynamic list */
int wimey_add_command(struct wimey_command_t cmd) {
	struct __wimey_command_node *new_cmd = 
		wimey_create_command_node(cmd);

	if(!new_cmd) {
		ERR("Failed to push back new command, invalid argument.");
		return WIMEY_ERR;
	}

    	if(wimey_dict.cmds_head == NULL) {
		wimey_dict.cmds_head = new_cmd;
		return WIMEY_OK;
	}

	struct __wimey_command_node *current = wimey_dict.cmds_head;

	while(current->next != NULL) {
		current = current->next;
	}
    	
	current->next = new_cmd;
	return WIMEY_OK;
}

/* This function returns the head of the commands list
 * or NULL if the list is empty  */
struct __wimey_command_node *wimey_get_commands_head(void) {
	return wimey_dict.cmds_head;
}

int wimey_init(void) {
	wimey_dict.cmds_head = NULL;
	return WIMEY_OK;
}

/* This function free all the lists  */
void wimey_free_all(void) {
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

void command_hello(const char *value) {
	printf("Hello command executed with value: %s\n", value ? value : "(no value)");
}

void command_goodbye(const char *value) {
	printf("Goodbye command executed with value: %s\n", value ? value : "(no value)");
}

int main(int argc, char **argv) {
	ERR("This is an error %s", "Invalid input");
	WARN("This is a warn %s", "don't ignore me");
	INFO("This is an info message %s", "you can ignore me");

	if (wimey_init() != WIMEY_OK) {
        	ERR("Failed to initialize Wimey");
        	return 1;
    	}

	struct wimey_command_t cmd1 = {
        	.key = "hello",
        	.has_value = false,
        	.is_value_required = false,
        	.value_name = NULL,
        	.callback = command_hello
    	};

    	struct wimey_command_t cmd2 = {
        	.key = "goodbye",
        	.has_value = false,
        	.is_value_required = false,
        	.value_name = NULL,
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

	wimey_free_all();

	return 0;
}

#endif
