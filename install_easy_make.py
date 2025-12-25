#!/usr/bin/env python3

import os
import platform
import shutil
import sys
import tempfile
import textwrap
import urllib.request
import urllib.error

RED_ANSI = "\033[31m"
RESET_ANSI = "\033[0m"


def get_version() -> str:
    VALID_VERSIONS = [
        "1.0.0",
        "1.1.0",
        "1.2.0",
    ]

    num_of_arguments = len(sys.argv) - 1
    too_many_arguments = num_of_arguments > 1
    if too_many_arguments:
        raise RuntimeError(f"Too many arguments: Expected at most 1, but got {num_of_arguments}.")

    version = VALID_VERSIONS[-1] if num_of_arguments == 0 else sys.argv[-1]

    if version not in VALID_VERSIONS:
        print(f"{RED_ANSI}Error: Version {version} is invalid.{RESET_ANSI}")
        print(f"Valid versions are:")

        for index, version in enumerate(reversed(VALID_VERSIONS)):
            is_last_version = index == 0
            if is_last_version:
                print(f"* {version} (latest)")
            else:
                print(f"* {version}")

        print("Not specifying a version selects the latest one.")
        sys.exit(1)

    return version


def get_os_and_architecture() -> tuple[str, str]:
    current_os = platform.system().lower()
    current_architecture = platform.machine().lower()

    if current_architecture == "x86_64":
        current_architecture = "x86-64"

    SUPPORTED_PLATFORMS = {("linux", "x86-64")}

    current_platform_is_supported = (current_os, current_architecture) in SUPPORTED_PLATFORMS
    if not current_platform_is_supported:
        print(f"{RED_ANSI}Error: Current platform is unsupported.{RESET_ANSI}")
        print(f"Current OS is {current_os}.")
        print(f"current architecture is {current_architecture}")
        print("Supported platforms are:")

        for os, architecture in SUPPORTED_PLATFORMS:
            print(f"* OS: {os}, architecture: {architecture}")

        sys.exit(1)

    return current_os, current_architecture


if __name__ == "__main__":
    version = get_version()
    current_os, current_architecture = get_os_and_architecture()
    filename = f"easy-make-{version}-{current_os}-{current_architecture}"
    url = f"https://github.com/idanhalp/easy-make/releases/download/v{version}/{filename}"

    # Download
    print(f"Downloading {filename}...")
    print(f"URL: {url}")

    try:
        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            with urllib.request.urlopen(url) as response:
                shutil.copyfileobj(response, tmp)
            downloaded_path = tmp.name
    except urllib.error.HTTPError as e:
        print(f"Download failed: {e}")
        sys.exit(1)
    except urllib.error.URLError as e:
        print(f"Network error: {e}")
        sys.exit(1)

    # Installation
    install_dir = "/usr/local/bin"
    target_path = os.path.join(install_dir, "easy-make")

    try:
        os.chmod(downloaded_path, 0o755)  # rwxr-xr-x
        shutil.move(downloaded_path, target_path)
    except PermissionError:
        print(
            textwrap.dedent(
                f"""
                {RED_ANSI}Error: Permission denied while installing.{RESET_ANSI}
                Try the following methods to solve the problem:
                1. Run the script with sudo: `sudo ./install_easy_make <version>`.
                2. Change the file's permissions: `chmod +x install_easy_make.py`.
                """
            ).strip()
        )

        os.remove(downloaded_path)
        sys.exit(1)
    except Exception as e:
        print(f"Installation failed: {e}")
        os.remove(downloaded_path)
        sys.exit(1)

    print("easy-make installed successfully!")
    print(f"Location: {target_path}")
    print("Run `easy-make --version` to verify the installation.")
