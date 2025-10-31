# easy-make

**easy-make** is a simple and friendly build tool for C++.  
It helps you build your project using a clear, easy-to-read configuration file without any messy makefiles, just straightforward builds.

![example-output](/documentation/assets/example-output.png)

## How to Use

- Create a `easy-make-configurations.json` at the root directory of the project.
- Define the relevant configuration. For example:

  ```json
  [
    {
      "name": "example-config",
      "compiler": "g++",
      "optimization": "0",
      "sources": {
        "files": ["main.cpp", "source/file_1.cpp", "source/file_2.cpp"],
        "directories": ["./utils"]
      },
      "output": {
        "name": "output.exe",
        "path": "build"
      }
    }
  ]
  ```

  This defines a configuration with the three specified files and the files inside the `utils` directory.

- Build the project using `easy-make <configuration-name>`. This will create an executable with the defined name at the defined path.  
  In our example, running `easy-make example-config` will create an executable called `output.exe` inside a directory called `build`.

## Features

- **Simple Setup:** All your sources, excludes, and flags live in one easy JSON file.
- **Automatic Builds:** Just describe your project once, and `easy-make` takes care of the rest.
- **Smart Rebuilds:** Only recompiles what changed, saving you time and frustration.
- **Built-in Safety Checks:** Warns you if a file or folder is missing before things break.
- **Custom Flags Made Easy:** Add defines, include paths, and extra compiler options effortlessly.
- **Clean Output:** Clear, minimal logs so you can focus on your code and not worry abut your build system.
