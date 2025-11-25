#!/usr/bin/env python3

import os
import subprocess

clang_format_is_installed = subprocess.run("clang-format --version", shell=True, capture_output=True).returncode == 0
assert clang_format_is_installed

root_directory_is_correct = os.path.isfile("easy-make-configurations.json")
assert root_directory_is_correct

root = os.getcwd()
directories_to_format = [
    "source",
    "tests",
]

for directory_to_format in directories_to_format:
    path_to_directory = os.path.join(root, directory_to_format)
    for dirpath, dirnames, filenames in os.walk(path_to_directory):
        for file in filenames:
            is_source_file = file.endswith(".cpp")
            is_header_file = file.endswith(".hpp")

            if is_source_file or is_header_file:
                command = f"clang-format -i {os.path.join(dirpath, file)}"
                output = subprocess.run(command, shell=True, text=True, capture_output=True)

                print(output.args)
                if output.stdout:
                    print(output.stdout)
                if output.stderr:
                    print(output.stderr)
