# WIMEY

**Wimey** is a simple-to-use C library to parse commands and arguments, written in **less <600 Lines of Code**. It supports both command and argument parsing, including value handling, automatic help generation, and type-safe conversions. Designed for flexibility and minimal dependencies, Wimey helps you structure your CLI programs cleanly and efficiently.

## Features

- 🧭 Command and subcommand support
- 🧾 Positional and optional arguments
- 🧠 Type-safe argument parsing (int, bool, string, float)
- 📖 Built-in help message generation
- 🧱 No external dependencies
- 🛠️ Simple, clean API for building CLI tools

## How to use

```c
#include <stdio.h>
#include "wimey.h"

int main(int argc, char **argv) {
    // Init Wimey
    if (wimey_init() != WIMEY_OK) {
        ERR("Init failed");
        return 1;
    }
    // Config
    struct wimey_config_t cfg = {
        .name = "Example",
        .description = "Minimal Wimey example",
        .version = "0.1",
        .log_level = LOG_ALL
    };
    wimey_set_config(&cfg);

    // Add inline command
    wimey_add_command((struct wimey_command_t){
        .key = "hello",
        .has_value = true,
        .is_value_required = true,
        .value_name = "name",
        .desc = "Say hello",
        .callback = [](const char *val) {
            INFO("Hello, %s!", val);
        }
    });
    // Parse args
    wimey_parse(argc, argv);
    // Clean up
    wimey_free_all();
    return 0;
}

```

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

* Inlude header file``
  
  ```c
  /* my_file.c */
  #include "wimey.h"
  
  // or with -Iinclude
  #include <wimey.h>
  ```

## Contributions

Please open a [pull request](https://github.com/UsboKirishima/wimey/pulls) or [issue](https://github.com/UsboKirishima/wimey/issues), thanks!

## License

Library under [Apache 2.0 License](LICENSE)
Copyright (C) 2025 Davide Usberti <usbertibox@gmail.com>
