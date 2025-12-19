# `init` Command Documentation

## Summary

Creates an `easy-make-configurations.json` file with a default configuration template.

## Usage

```
easy-make init [options]
```

## Behavior

- Checks whether an `easy-make-configurations.json` file exists in the project root.
- If the file does not exist:
  - Creates a new configuration file containing a default example configuration.
- If the file already exists:
  - The command fails unless `--overwrite` is specified.
- When `--overwrite` is specified:
  - The existing configuration file is replaced with the default template.
- Status messages are printed unless `--quiet` is specified.

## Options

- `--overwrite`  
  Overwrite an existing `easy-make-configurations.json` file.

- `--quiet`  
  Suppress informational output.

## Exit Status

- `0`  
  The configuration file was successfully created or overwritten.

- `1`  
  The command failed because of one of the following reasons:
  - Invalid arguments were supplied.
  - The configuration file already exists and `--overwrite` was not specified.

## Examples

```
easy-make init
easy-make init --overwrite
easy-make init --quiet
```
