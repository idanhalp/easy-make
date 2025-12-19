# `list-files` Command Documentation

## Summary

Lists the source and header files included in a configuration.

## Usage

```
easy-make list-files <configuration-name> [options]
```

## Behavior

- Resolves the specified configuration by name.
- If the configuration does not exist or is invalid, the command fails.
- Collects all source and header files included in the configuration.
- Files are sorted lexicographically before being printed.
- Prints information about the relevant files depending on the specified options. By default, prints a verbose, human-readable list of files.

## Options

- `--count`  
  Print the number of files after applying any file-type filters.

- `--header-only`  
  Include only header files when selecting files from the configuration.  
  This option is mutually exclusive with `--source-only`.

- `--porcelain`  
  Print only file names, with no additional formatting.

- `--source-only`  
  Include only source files when selecting files from the configuration.  
  This option is mutually exclusive with `--header-only`.

## Exit Status

- `0`  
  The command completed successfully.

- `1`  
  The command failed because of one of the following reasons:
  - Invalid arguments were supplied.
  - The specified configuration does not exist or is invalid.

## Examples

```
easy-make list-files debug
easy-make list-files debug --count
easy-make list-files debug --header-only
easy-make list-files debug --source-only --porcelain
```
