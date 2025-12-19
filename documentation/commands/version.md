# `version` Command Documentation

## Summary

Prints the currently installed **easy-make** version.

## Usage

```
easy-make version
```

## Behavior

- Prints the current easy-make version in `MAJOR.MINOR.PATCH` format.
- The version is compiled into the binary.
- The command does not accept any arguments.
- If invalid arguments are supplied, the command fails.

## Options

This command does not support any options.

## Exit Status

- `0`  
  The version was printed successfully.

- `1`  
  The command failed because of one of the following reasons:
  - Invalid arguments were supplied.

## Examples

```
easy-make version
```
