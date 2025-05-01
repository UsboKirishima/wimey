# wimey
Wimey - C library to handle command line tool easily

### What's Wimey?

**Wimey** is a simple-to-use C library to parse commands and arguments, written in **less <600 Lines of Code**.

### What can you do

With Wimey you can:
* Add new commands
* Add arguments
* Generate help command (soon)
* Keep a linked list of you cmds/args
* Keep your project clean & readable

### How to use

Please read the [full example here](example.c).

#### In your project

* Clone repo:
    ```bash
    git clone https://github.com/UsboKirishima/wimey && cd wimey
    ```
* Move file to your project:
    ```bash
    mv wimey.c ~/coding/myproj
    mv wimey.h ~/coding/myproj
    ```

* Inlude header file:
    ```c
    /* my_file.c */
    #include "wimey.h"

    // or with -linclude
    #include <wimey.h>
    ```
### Contribute

Please open a [pull request](https://github.com/UsboKirishima/wimey/pulls) or [issue](https://github.com/UsboKirishima/wimey/issues)

Thanks!

### License

Library under [Apache 2.0 License](LICENSE) \
Copyright (C) 2025 Davide Usberti <usbertibox@gmail.com>