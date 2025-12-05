# Commands

This document specifies the commands which can be supplied to the program.

| Command        | Description                                                                     | Parameters         |
| -------------- | ------------------------------------------------------------------------------- | ------------------ |
| `build`        | Builds the specified configuration.                                             | configuration name |
| `clean`        | Removes the executable and object files related to the specified configuration. | configuration name |
| `clean-all`    | Removes all the executables and object files.                                   | none               |
| `init`         | Creates a new `easy-make-configurations.json` file                              | none               |
| `list-configs` | Lists the configurations in the `easy-make-configurations.json` file            | none               |
| `list-files`   | Lists the files in a configuration.                                             | configuration name |
| `version`      | Prints the program version.                                                     | none               |

## Flags

### `build`

- `--parallel`: Compile multiple files simultaneously in batches to improve build performance.
- `--quiet`: Prevents printing information. Compiler warning and errors are still printed.

### `clean`

- `--quiet`: Prevents printing information.

### `clean-all`

- `--quiet`: Prevents printing information.

### `init`

- `--quiet`: Prevents printing information.
- `--overwrite`: Overwrite the `easy-make-configurations.json` file if it exists.

### `list-configs`

- `--complete-only`: Print only the names of complete configurations.
- `--count`: Print only the number of relevant configurations.
- `--incomplete-only`: Print only the names of incomplete configurations.
- `--porcelain`: Print only configuration names, with no additional formatting.
- `--sorted`: Print configurations in lexicographical order.  
  If not provided, configurations are printed in the order they appear in the `easy-make-configurations.json` file.

### `list-files`

- `--count`: Print only the number of files in the configuration.
- `--header-only`: Print only the names of header files in the configuration.
- `--porcelain`: Print only file names, with no additional formatting.
- `--source-only`: Print only the names of source files in the configuration.
