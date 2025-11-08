# How to Use easy-make

This guide explains how to use **easy-make** to build your C++ projects from a JSON specification.

## 1. Prepare Your Project

1. Create a project.
2. Put the JSON configuration file `easy-make-configurations.json` in the project root.

For example:

```
my_project/
├── src/
|   ├── main.cpp
│   ├── file_1.cpp
│   ├── file_2.cpp
│   └── directory_1/
│       ├── file_3.cpp
│       └── file_4.cpp
├── tests/
|   ├── test_main.cpp
│   ├── test_1.cpp
│   └── test_2.cpp
└── easy-make-configurations.json
```

## 2. Create the JSON Configuration

Your JSON should define configurations for compilation.

For example, a `easy-make-configurations.json` file for the aforementioned project could look like this:

```json
[
  {
    "name": "base",
    "compiler": "g++",
    "standard": "23",
    "warnings": ["-Wall", "-Wextra"],
    "sources": {
      "directories": ["."] // Include all the source files under the root.
    },
    "output": { "path": "build" }
  },
  {
    "name": "test",
    "parent": "base",
    "optimization": "0",
    "exclude": {
      "files": ["src/main.cpp"] // We want to run `tests/test_main.cpp` instead.
    },
    "output": { "name": "my-app-test" }
  },
  {
    "name": "release",
    "parent": "base",
    "optimization": "3",
    "exclude": {
      "directories": ["test"]
    },
    "warnings": ["-Wall", "-Wextra", "-Werror"],
    "output": { "name": "my-app-release" }
  }
]
```

## 3. Run easy-make

From the project root, run:

```
easy-make build <configuration_name>
```

Examples:

```
easy-make build test
easy-make build release
```

easy-make will create an executable in the specified output path.

## 4. Notes

For details about what each key does, see [json_configurations.md](json-configurations.md).
