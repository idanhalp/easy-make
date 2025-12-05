#!/usr/bin/env python3

import subprocess
import sys

too_many_arguments = len(sys.argv) > 2

if too_many_arguments:
    print("Too many arguments; expected 0 or 1.")
    sys.exit(1)

compilation_successful = (
    subprocess.run("./easy-make build debug", shell=True).returncode == 0
    and subprocess.run("./easy-make build test", shell=True).returncode == 0
)

if not compilation_successful:
    print("Compilation of 'debug' and 'test' configurations failed.")
    sys.exit(1)

argument = sys.argv[1] if len(sys.argv) == 2 else None

match argument:
    case None:
        print("Running all tests.")
        subprocess.run("./test", shell=True)
    case "p":
        print("Running performance tests.")
        subprocess.run('./test --test-case="*[performance]*"', shell=True)
    case "r":
        print("Running regression tests.")
        subprocess.run("./test --test-suite=regression", shell=True)
    case "u":
        print("Running unit tests.")
        subprocess.run("./test --test-suite=unit", shell=True)
    case _:
        print(f"Unknown argument '{argument}'. Valid arguments are: {["r", "u"]}.")
        sys.exit(1)
