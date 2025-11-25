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
