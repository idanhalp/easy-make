#!/usr/bin/env python3
import subprocess
import sys


def get_arguments() -> tuple[int, int, int]:
    assert len(sys.argv) == 2

    values = sys.argv[1].split(".")
    assert len(values) == 3

    major, minor, patch = values
    assert major.isdigit()
    assert minor.isdigit()
    assert patch.isdigit()

    return int(major), int(minor), int(patch)


if __name__ == "__main__":
    original_branch = subprocess.run(
        "git branch --show-current", shell=True, capture_output=True, text=True
    ).stdout.strip()

    major, minor, patch = get_arguments()
    version = f"{major}.{minor}.{patch}"
    tag = f"v{version}"
    tag_exists = (
        subprocess.run(f'git rev-parse -q --verify "refs/tags/{tag}"', shell=True, capture_output=True).returncode == 0
    )

    if not tag_exists:
        print(f"Tag '{tag}' does not exists. Cannot compile version.")
        sys.exit(1)

    print(f"Building version {version}...")
    subprocess.run(f"git switch --detach {tag}", shell=True, capture_output=False)
    subprocess.run("easy-make clean-all", shell=True, capture_output=True)
    subprocess.run("easy-make build release --parallel", shell=True, capture_output=False)

    print('Moving executable to "Downloads" folder in Windows...')
    new_executable_name = f"easy-make-{version}-linux-x86-64"
    subprocess.run(f"mv easy-make {new_executable_name}", shell=True, capture_output=True)
    subprocess.run(f"cp {new_executable_name} /mnt/c/Users/USER/Downloads/", shell=True, capture_output=True)

    print("Cleaning up...")
    subprocess.run(f"rm {new_executable_name}", shell=True, capture_output=True)
    subprocess.run(f"git switch {original_branch}", shell=True, capture_output=False)

    print("Done creating new version executable!")
