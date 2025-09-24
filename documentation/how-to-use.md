# How to Use easy-make

This guide explains how to use easy-make to build your C++ projects from a JSON specification.

## 1. Prepare Your Project

1. Organize your project with a source folder (for example, src/) and put all .cpp files there.  
2. Create the JSON configuration file easy-make-configurations.json in the project root.

Example project structure:

```
my_project/  
├── src/  
│   ├── file_1.cpp  
│   ├── file_2.cpp  
│   └── directory_1/  
│       ├── file_3.cpp  
│       └── file_4.cpp  
└── easy-make-configurations.json  
```

## 2. Create the JSON Configuration

Your JSON should define:

- default: global defaults for compiler, standard, warnings, optimization, files, output.  
- configurations: one or more named build configurations.  

Default values are overridden when a configuration specifies something different.

Example configuration:

```json
[
  { 
    "name": "default",
    "compiler": "g++",
    "standard": "c++23",
    "warnings": ["-Wall", "-Wextra"],
    "optimization": "-O2",
    "sources": {
      "files": ["src/main.cpp", "src/utils.cpp"],
    },
    "output": { "path": "build", "name": "my_app" }
  },
  {
    "name": "debug",
    "optimization": "-O0",
    "warnings": ["-Wall", "-Wextra", "-Wpedantic"],
    "output": { "path": "build/debug" }
  },
  {
    "name": "release",
    "optimization": "-O3",
    "output": { "path": "build/release" }
  }
]
```

## 3. Run easy-make

From the project root, run:

```bash
easy-make <configuration_name>  
```

Examples:  

```bash
easy-make debug  
easy-make release  
```

Running `easy-make` without arguments will use the default configuration.  

easy-make will create an executable in the specified output path.  

## 4. Notes

- Always use JSON arrays for lists like files, warnings, or defines.  
- Paths are relative to the JSON file: both files and output.path are resolved relative to `easy-make-configurations.json`.

Next, for full details about all fields, merge rules, and advanced usage, see [json_configurations.md](json-configurations.md).  
