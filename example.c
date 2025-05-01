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
#include <stdint.h>

#include "wimey.h"

#define true 1
#define false 0

#define SQUARE(x) (x * x)

/* --------- Command Declarations --------- */

void command_hello(const char *value);
void command_square(const char *value);

/* --------- Command Definitions --------- */

struct wimey_command_t cmd1 = {
	.key = "hello",
	.has_value = true,
	.is_value_required = true,
	.value_name = "Name",
	.callback = command_hello
};

struct wimey_command_t cmd2 = {
	.key = "square",
	.has_value = true,
	.is_value_required = true,
	.value_name = "Number (double)",
	.callback = command_square
};

/* ------------- Argument Definitions -------------- */
int version = false;
long count = 0;

struct wimey_argument_t arg1 = {
	.long_key = "--version",
	.short_key = "-v",
	.has_value = false,
	.is_value_required = false,
	.value_dest = &version,
	.value_name = NULL,
	.value_type = WIMEY_BOOL,
	.desc = "Show version of the program"
};

struct wimey_argument_t arg2 = {
	.long_key = "--count",
	.short_key = "-c",
	.has_value = true,
	.is_value_required = true,
	.value_dest = &count,
	.value_name = "Number",
	.value_type = WIMEY_LONG,
	.desc = "Count util the number value"
};

/* --------- Callback Implementations --------- */

void command_hello(const char *value)
{
	INFO("Hello: %s", value ? value : "(no value)");
}

void command_square(const char *value) /* square of number */
{
	if(!value) return;
	double res = SQUARE(wimey_val_to_double(value));
	INFO("%s ^ 2 = %.2f", value, res);
}

int main(int argc, char **argv)
{	
	/* 1. First, we need to call wimey_init(),
	 *    which initializes default values in structs and
	 *    does some other setup work.
	 */
	if (wimey_init() != WIMEY_OK) {
		ERR("Failed to initialize Wimey");
		return 1;
	}

	/* 2. Optionally, we can customize some library settings
	 *    by creating a wimey_config_t struct with our values.
	 *    To apply the configuration, call wimey_set_config().
	 *
	 *    NOTE: myconf.log_level accepts an integer value,
	 *    the flags avaible for this:
	 *
	 * 	LOG_ERR_ONLY - Errors only
 	 * 	LOG_ERR_AND_WARNS - Errors & Warns
 	 * 	LOG_ALL - Errors, Warns and Info 
	 */
	struct wimey_config_t myconf = {
		.log_level = LOG_ALL, 
		.name = "Example CLI",
		.description = "Simple example using the Wimey library",
		.version = "1.0.0"	
	};

	wimey_set_config(&myconf);

	/* !! THIS IS AN ERROR !! */
	/* myconf.log_level = LOG_ERR_ONLY; */

	/* 3. Add each command to the internal global command list,
	 *    and check if the library successfully allocated it
	 *    using simple error checking.
	 *
	 *    NOTE: Wimey also provides logging macros that you
	 *    can use in your own program:
	 *
	 *    	ERR(str, ...)  - For fatal errors
	 *    	WARN(str, ...) - For warnings
	 *    	INFO(str, ...) - For general logs
	 */
	if (wimey_add_command(cmd1) != WIMEY_OK) {
		ERR("Failed to add command: %s", cmd1.key);
	}

	if (wimey_add_command(cmd2) != WIMEY_OK) {
		ERR("Failed to add command: %s", cmd2.key);
	}

	if(wimey_add_argument(arg1) != WIMEY_OK) {
		ERR("Failed to add argument: %s", arg1.long_key);
	}

	if(wimey_add_argument(arg2) != WIMEY_OK) {
		ERR("Failed to add argument: %s", arg2.long_key);
	}
	
	/* 4. You can explore and iterate over the internal command list.
	 *    For example, you can loop through and print all the
	 *    available commands!
	 */
	struct __wimey_command_node *current = wimey_get_commands_head();
	
	INFO("Commands list: ");
	while (current != NULL) {
		printf("	- %s | %s\n", current->cmd.key, 
				current->cmd.value_name);
		current = current->next;
	}
	
	struct __wimey_argument_node *current_arg = wimey_get_arguments_head();

	INFO("Arguments list: ");
	while (current_arg != NULL) {
		printf("	- %s | %s\n", current_arg->argument.long_key, 
				current_arg->argument.value_name);
		current_arg = current_arg->next;
	}

	INFO("");

	/* 5. After initializing commands, arguments, and configuration,
	 *    call wimey_parse(argc, argv) to process the command-line
	 *    arguments and trigger the corresponding callbacks.
	 */
	wimey_parse(argc, argv);

	if(version)
		INFO("Version 1.0.0");

	INFO("The value of count is %d",(int)count);
	
	/* 6. Call this function at the end to clean up
	 *    all allocated memory easily.
	 */
	wimey_free_all();

	return 0;
}

