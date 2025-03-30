# c_minilib_config

`c_minilib_config` is a lightweight, modular C library designed to simplify application configuration management. It enables your application to effortlessly support multiple configuration formats (`.env`, `.json`, `.yaml`) using a unified, generic interface.

## Features

- **Unified Interface:** Seamlessly handle `.env`, `.json`, and `.yaml` files through one consistent C struct.
- **Flexible Configuration Path:** Set your configuration file location dynamically via an environment variable.
- **Minimal Dependencies:** Lightweight, easy to integrate, and designed for minimalist projects.
- **Modular by Design:** Supports easy extension or customization for additional configuration formats.

## Getting Started

### Usage Example

```c
#include "c_minilib_config.h"

int main(void) {
    config_t config;
    
    if (config_load(&config) != CONFIG_SUCCESS) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    printf("Server host: %s\n", config.server_host);
    printf("Server port: %d\n", config.server_port);

    config_free(&config);
    return 0;
}
```

### Configuration File Location

By default, `c_minilib_config` searches for configuration files at a predefined location. You can override this by setting the `CONFIG_PATH` environment variable:

```shell
export CONFIG_PATH=/path/to/config.yaml
```

## License

`c_minilib_config` is distributed under the MIT license.

