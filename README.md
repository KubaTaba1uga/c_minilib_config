# c_minilib_config

**`c_minilib_config`** is a minimalist, extensible configuration parser written in C. It provides a unified, tree-based API for reading structured application configuration from `.env` files, with plans for future support of `.json` and `.yaml`.

## ✨ Features

- **Structured Tree Representation**: Each config is parsed into a tree of typed fields (`int`, `string`, `array`, `dict`), supporting deeply nested configurations.
- **Environment Variable Parsing**: Reads `.env` files using compound key syntax for arrays and dicts (e.g. `ARRAY_0=value`, `DICT_KEY=value`).
- **Macro-Driven Iteration**: Convenient macros like `CMC_FOREACH_FIELD_ARRAY` simplify traversal of arrays and dictionaries.
- **Strict Type Safety**: All field types are declared up front, and parsing validates types and presence.
- **Zero External Dependencies**: Lightweight and embeddable in any C project.
- **Extensible Design**: Built with parser plugin support—additional formats like JSON/YAML can be plugged in.
- **Fully Tested**: Includes comprehensive tests using [Unity](https://www.throwtheswitch.org/unity) framework.

## 🧠 Example Usage

```c
#include "c_minilib_config.h"

int main(void) {
    cmc_error_t err = cmc_lib_init();
    if (err) return 1;

    struct cmc_Config *config;
    char *paths[] = {"/etc/myapp"};
    err = cmc_config_create(&(struct cmc_ConfigSettings){
        .supported_paths = paths,
        .paths_length = 1,
        .name = "app",
        .log_func = NULL,
    }, &config);
    if (err) return 1;

    struct cmc_ConfigField *field;
    err = cmc_field_create("host", cmc_ConfigFieldTypeEnum_STRING, "localhost", true, &field);
    cmc_config_add_field(field, config);

    err = cmc_config_parse(config);
    if (err) return 1;

    char *out = NULL;
    cmc_field_get_str(field, &out);
    printf("Host: %s\n", out);

    cmc_config_destroy(&config);
    cmc_lib_destroy();
    return 0;
}
```

## ⚙️ Build Instructions

Using [Meson](https://mesonbuild.com):

```sh
meson setup build
meson compile -C build
```

## ✅ Run Tests

```sh
meson test -C build
```

> Tests are written with [Unity](https://www.throwtheswitch.org/unity) and include full coverage of parsing logic, nested data, optional/required fields, and type validation.

## 🧰 Development Tools

Automated workflows are managed via [Invoke](https://www.pyinvoke.org):

```sh
inv install    # Install required packages
inv build      # Configure & compile
inv test       # Run all tests
inv lint       # Static analysis (clang-tidy)
inv format     # Format code (clang-format)
inv clean      # Clean build & temporary files
```

## 📚 Configuration Formats

Supported (via plugins):

- ✅ `.env` — flat key=value pairs with compound keys for nesting.
- 🚧 `.json`, `.yaml` — support planned, pluggable parser interface is ready.

## 🔍 Supported Field Types

- `STRING`: key-value pair
- `INT`: numeric configuration
- `ARRAY`: homogeneous lists (`KEY_0`, `KEY_1`, ...)
- `DICT`: nested key-value maps (`PARENT_CHILD=value`)

Example `.env` for a nested config:

```env
SERVER_HOST=localhost
SERVER_PORT=8080
USERS_0_NAME=alice
USERS_0_ROLE=admin
USERS_1_NAME=bob
USERS_1_ROLE=user
```

## 🧪 Debugging

Enable detailed parser logs by providing a custom logger in `ConfigSettings`. You can also inspect values using `cmc_field_get_str`, `cmc_field_get_int`, and iteration macros.

## 📄 License

MIT License. See [LICENSE](LICENSE) for full text.

