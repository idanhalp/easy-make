# `list-configs` Command Documentation

## Summary

Lists the configurations defined in the `easy-make-configurations.json` file.

## Usage

```
easy-make list-configs [options]
```

## Behavior

- By default, prints all configurations defined in `easy-make-configurations.json`.
- Output may include additional formatting unless modified by options.

## Options

- `--complete-only`  
  Print only the names of complete configurations.

- `--count`  
  Print only the number of relevant configurations.

- `--incomplete-only`  
  Print only the names of incomplete configurations.

- `--porcelain`  
  Print only configuration names, with no additional formatting.

- `--sorted`  
  Print configurations in lexicographical order.  
  If not provided, configurations are printed in the order they appear in the
  `easy-make-configurations.json` file.

## Exit Status

- `0`  
  The command completed successfully.

- `1`  
  Invalid arguments were provided.

## Examples

```
easy-make list-configs
easy-make list-configs --complete-only
easy-make list-configs --incomplete-only --sorted
easy-make list-configs --count
```
