# easy-make JSON Specifications

This document explains the structure and fields of the easy-make JSON configuration file (`easy-make-configurations.json`).  
It serves as a complete reference for creating and customizing easy-make builds.

## 1. File Structure

The JSON file has two main sections:

default:  
  global default values

configurations:  
  <config_name>:  
    configuration-specific overrides

- **default:** contains global default values applied to all configurations.  
- **configurations:** defines one or more named build configurations. You can choose any names (for example, debug, release, profiling).  

## 2. Fields

* `name`:
  * Name of the configuration.
  * **default** is reserved for the default configuration.
  * Example:
    ```json
    "name": "release"
    ```

* `compiler`  
  * C++ compiler.  
  * Options: 
    * g++
    * clang++
    * cl  
  * Example:
    ```json
    "compiler": "g++"
    ```

* `standard`  
  * C++ standard.  
  * Options: 
    * c++98
    * c++03
    * c++11
    * c++14
    * c++17
    * c++20
    * c++23
    * c++26  
  * Example:
    ```json
    "standard": "c++20"
    ```

* `warnings`  
  * Compiler warning flags.  
  * Example: 
    ```json 
    "warnings": ["-Wall", "-Wextra", "-Wpedantic"]
    ```

* `optimization`  
  * Compiler optimization flag.  
  * Options: 
    * -O0
    * -O1
    * -O2
    * -O3
    * -Ofast
    * -Os
    * /Od
    * /O1
    * /O2
    * /Ox  
  * Example:
    ```json
    "optimization": "-O2"
    ```

* `defines`  
  * Preprocessor defines.  
  * Example:  
    "defines": ["-DDEBUG=1"]

* `sources`  
  * files: list of files to compile.  
  * directories: list of directories to recursively scan and include all source files within. Supports .cpp, .cc, .cxx files.  
  * All paths are relative to the JSON file.  
  * Duplicate files are allowed.  
  * Example:
    ```json
    "sources": {  
      "files": ["src/dir_1/file_1.cpp", "src/dir_2/file_2.cpp"],
      "directories": ["src/dir_3", "src/dir_4"]
    }
    ```
* `exclude`  
  * `files`: list of files to exclude.  
  * `directories`: list of directories to recursively exclude.  
  * Exclusions take precedence over inclusions in sources.  
  * Example:
    ```json  
    "exclude": {  
      "files": ["src/dir_1/file_1.cpp", "src/dir_2/file_2.cpp"],  
      "directories": ["src/dir_3", "src/dir_4"]  
    }
    ```

* output  
  * Target executable information. Must have:  
    * `name`: name of the executable  
    * `path`: path to put the executable, relative to the JSON file  
  * Example:
    ```json 
    "output": { "name": "output.exe", "path": "build/release" }
    ```
## 3. Configurations

- `easy-make` supports several configurations in one `.json` file.
- The default configuration has **default** for its `name` field.
- Each configuration can override any default field.  
- Configurations are optional; if none is specified, easy-make uses the default configuration.  
- Example:  
  ```json
    {  
      "name": "debug",
      "optimization": "-O0",  
      "warnings": ["-Wall", "-Wextra", "-Wpedantic"],  
      "output": { "name": "debug.exe", "path": "build/debug" }  
    }
    {  
      "name": "release",
      "optimization": "-O3",  
      "warnings": ["-Wall", "-Wextra", "-Wpedantic"],  
      "output": { "name": "release.exe", "path": "build/release" }  
    }
  ```

## 4. Merge Rules

When easy-make reads a configuration, it overrides the default value.

Example:  

```json
{
  "name": "default",  
  "compiler": "g++",  
  "files": ["foo.cpp"],  
  "optimization": "-O2"  
},
{
  "name": "debug", 
  "optimization": "-O0"
}  
```

Running `easy-make debug` uses `-O0` optimization, while running `easy-make` uses the default `-O2`.
