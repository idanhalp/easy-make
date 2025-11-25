#!/usr/bin/env python3

import subprocess

subprocess.run("make clean", check=True, shell=True)
subprocess.run("make easy-make", check=True, shell=True)
subprocess.run("sudo cp easy-make /usr/local/bin/easy-make", check=True, shell=True)
subprocess.run("sudo chmod +x /usr/local/bin/easy-make", check=True, shell=True)
subprocess.run("easy-make version", check=True, shell=True)
