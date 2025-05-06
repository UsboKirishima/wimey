# wimey

Wimey - C library to handle command line tool easily

## What's Wimey?

**Wimey** is a simple-to-use C library to parse commands and arguments, written in **less <600 Lines of Code**.

## What can you do

With Wimey you can:

* Add new commands
* Add arguments
* Generate help command
* Keep a linked list of you cmds/args
* Keep your project clean & readable

## How to use

Please read the [full example here](example.c).

### In your project

* Clone repo:
  
  ```bash
  git clone https://github.com/UsboKirishima/wimey && cd wimey
  ```

* Move file to your project:
  
  ```bash
  mv wimey.c ~/coding/myproj
  mv wimey.h ~/coding/myproj
  ```

* Inlude header file
  
  ```c
  /* my_file.c */
  #include "wimey.h"
  
  // or with -Iinclude
  #include <wimey.h>
  ```
  
  ## Reference
  
  ```c
  int wimey_init(void);
  int wimey_set_config(struct wimey_config_t *conf);
  struct wimey_config_t wimey_get_config(void);
  void wimey_free_all(void);
  int wimey_parse(int argc, char **argv);
  int wimey_add_command(struct wimey_command_t cmd);
  struct __wimey_command_node *wimey_get_commands_head(void);
  int wimey_add_argument(struct wimey_argument_t argument);
  struct __wimey_argument_node *wimey_get_arguments_head(void);
  ```
  
  ## Contribute

Please open a [pull request](https://github.com/UsboKirishima/wimey/pulls) or [issue](https://github.com/UsboKirishima/wimey/issues), thanks!

## License

Library under [Apache 2.0 License](LICENSE)
Copyright (C) 2025 Davide Usberti <usbertibox@gmail.com>
