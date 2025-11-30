# Regression Tests

## Bug #1

### Description

Check that compilation is successful when no flags are used.

### Original Cause

The function that creates the flag string (`create_compilation_flags_string`) wanted to trim whitespace and removed the last char, but did not check if the string is empty.

### Solution

Check if the string ends with whitespace before trimming.

## Bug #2

### Description

File is not recompiled after a compilation failure if an old object file exists.

### Original Cause

When determining whether to compile a file, we check whether its hash has changed or whether an object file already exists.
If a file was previously compiled successfully and then modified, it will be recompiled on the first attempt because the hash has changed. However, on the second attempt, the hash has already been updated to match the new file, so it will not be recompiled again.

### Solution

If a file fails to compile, delete the object file.

## Bug 3

### Description

When a file is removed, other files that depend on it aren't getting recompiled.

### Original Cause

We're only checking which existing files changed and recompiling what's affected by those changes. If a file gets deleted, nothing happens because there's no changed file to trigger recompilation.

### Solution

Keep track of the previous dependency graph. When a file disappears from the new graph, mark all the files that depended on it for recompilation.

## Bug 4

### Description

Program crashes when compiling a project where no file includes another (and the dependency graph is empty).

### Original Cause

When dumping an empty json, it appears as `null` and not as `{}`.

### Solution

If the dependency graph is empty delete the old data and don't dump anything.

## Bug 5

### Description

Program crashes after deleting a source file.

### Original Cause

Object files for deleted source files were not removed, leading the linker to use stale artifacts.

### Solution

Remove object files associated with deleted source files.
