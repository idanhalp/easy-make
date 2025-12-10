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


current_branch = subprocess.run("git branch --show-current", shell=True, capture_output=True, text=True).stdout.strip()
if current_branch != "development":
    print("Must be on branch 'development' when pushing new version!")
    sys.exit(1)

major, minor, patch = get_arguments()
version = f"{major}.{minor}.{patch}"
tag_name = f"v{version}"

tag_already_exists = (
    subprocess.run(f'git rev-parse -q --verify "refs/tags/{tag_name}"', shell=True, capture_output=True).returncode == 0
)

if tag_already_exists:
    print(f"Tag '{tag_name}' already exists!")
    sys.exit(1)

subprocess.run(f'git commit --allow-empty -m "Updated version to {version}."', shell=True)
subprocess.run(f"git commit --allow-empty --amend", shell=True)
subprocess.run("git push origin development", shell=True)
subprocess.run(f'git tag -a {tag_name} -m "Version {version}"', shell=True)
subprocess.run(f"git push origin {tag_name}", shell=True)
subprocess.run(f"git switch master", shell=True)
subprocess.run(f"git merge development", shell=True)
subprocess.run("git push origin master", shell=True)
