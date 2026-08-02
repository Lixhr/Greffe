"""Shared target/variant definitions for instrument.py and test_runtime.py."""

import os
from dataclasses import dataclass

TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(TESTS_DIR, "build")
PATCHED_DIR = os.path.join(BUILD_DIR, "patched")


@dataclass(frozen=True)
class Target:
    arch: str
    qemu: str
    force_arm: bool = False

    @property
    def binary(self):
        return os.path.join(BUILD_DIR, f"{self.arch}-test")

    @property
    def patched(self):
        return os.path.join(PATCHED_DIR, f"{self.arch}-test")


TARGETS = [
    Target("arm32", "qemu-arm", force_arm=True),
    Target("arm32-be", "qemu-armeb", force_arm=True),
    Target("arm32-thumb", "qemu-arm"),
    Target("arm32-thumb-be", "qemu-armeb"),
    Target("aarch64-le", "qemu-aarch64"),
    Target("aarch64-be", "qemu-aarch64_be"),
]
