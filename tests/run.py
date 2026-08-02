"""Builds the test binaries, instruments them with greffe, then runs them under qemu.

    IDA_DIR=/path/to/ida python3 run.py
"""

import subprocess
import sys

import instrument
import test_runtime
from targets import TESTS_DIR


DOCKER_IMAGE = "greffe-tests"


def build():
    print("=== building test binaries (docker) ===")
    proc = subprocess.run(["docker", "build", "-t", DOCKER_IMAGE, TESTS_DIR])
    if proc.returncode != 0:
        sys.exit("[!] docker build failed")

    proc = subprocess.run(
        ["docker", "run", "--rm", "-v", f"{TESTS_DIR}:/workspace", DOCKER_IMAGE]
    )
    if proc.returncode != 0:
        sys.exit("[!] build failed")


def main():
    build()

    print("\n=== instrumenting ===")
    if not instrument.run():
        sys.exit("[!] instrumentation failed")

    print("\n=== running ===")
    sys.exit(0 if test_runtime.run() else 1)


if __name__ == "__main__":
    main()
