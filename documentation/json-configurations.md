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
    "files": ["src/dir_1/file_1.cpp", "src/dir_2/file_2.cpp"]  
    "directories": ["src/dir_3", "src/dir_4"]
    ```
* exclude  
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

- Each configuration can override any default field.  
- Configurations are optional; if none is specified, easy-make uses the defaults.  
- Example:  
  ```json
  "configurations": {  
    "debug": {  
      "optimization": "-O0",  
      "warnings": ["-Wall", "-Wextra", "-Wpedantic"],  
      "output": { "name": "output.exe", "path": "build/debug" }  
    }  
  }
  ```

## 4. Merge Rules

When easy-make reads a configuration, it overrides the default value.

Example:  

```json
"default": {  
  "compiler": "g++",  
  "files": ["foo.cpp"],  
  "optimization": "-O2"  
}  

"configurations": {  
  "debug": { "optimization": "-O0" }  
}  
```

Running `easy-make debug` uses `-O0` optimization, while running `easy-make` uses the default `-O2`.
