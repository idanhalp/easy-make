# Commands

This document specifies the commands which can be supplied to the program.

| Command        | Description                                                                     | Parameters         |
| -------------- | ------------------------------------------------------------------------------- | ------------------ |
| `build`        | Builds the specified configuration.                                             | configuration name |
| `clean`        | Removes the executable and object files related to the specified configuration. | configuration name |
| `clean-all`    | Removes all the executables and object files.                                   | none               |
| `list-configs` | Lists the configurations in the `easy-make-configurations.json` file            | none               |
| `version`      | Prints the program version.                                                     | none               |

## Flags

### `clean`

- `--quiet`: Prevents printing information.

### `clean-all`

- `--quiet`: Prevents printing information.

### `list-configs`

- `--complete-only`: Print only the names of complete configurations.
- `--count`: Print only the number of relevant configurations.
- `--incomplete-only`: Print only the names of incomplete configurations.
- `--porcelain`: Print only configuration names, with no additional formatting.
- `--sorted`: Print configurations in lexicographical order.  
  If not provided, configurations are printed in the order they appear in the `easy-make-configurations.json` file.
