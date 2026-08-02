"""Runs every instrumented test binary under qemu-user and checks its output."""

import os
import subprocess
import sys
import unittest

from targets import TARGETS

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"
USE_COLOR = sys.stdout.isatty()


def colorize(text, color):
    return f"{color}{text}{RESET}" if USE_COLOR else text


class ColorResult(unittest.TestResult):
    def __init__(self, stream):
        super().__init__()
        self.stream = stream

    def startTest(self, test):
        super().startTest(test)
        self.stream.write(f"{test.shortDescription() or test.id()} ... ")
        self.stream.flush()

    def addSuccess(self, test):
        super().addSuccess(test)
        self.stream.write(colorize("PASS", GREEN) + "\n")

    def addFailure(self, test, err):
        super().addFailure(test, err)
        self.stream.write(colorize("FAIL", RED) + "\n")
        self.stream.write(self._exc_info_to_string(err, test) + "\n")

    def addError(self, test, err):
        super().addError(test, err)
        self.stream.write(colorize("ERROR", RED) + "\n")
        self.stream.write(self._exc_info_to_string(err, test) + "\n")


class PatchedBinaryTests(unittest.TestCase):
    pass


def make_test(target):
    def test(self):
        self.assertTrue(
            os.path.isfile(target.patched),
            f"patched binary not found: {target.patched} (run instrument.py first)",
        )
        result = subprocess.run(
            [target.qemu, target.patched],
            capture_output=True,
            text=True,
            timeout=30,
        )
        output = result.stdout + result.stderr
        self.assertIn("CRC OK", output, f"unexpected output:\n{output}")

    test.__doc__ = f"[{target.arch}] under {target.qemu}"
    return test


for _target in TARGETS:
    setattr(PatchedBinaryTests, f"test_{_target.arch.replace('-', '_')}", make_test(_target))


def run():
    suite = unittest.TestLoader().loadTestsFromTestCase(PatchedBinaryTests)
    result = ColorResult(sys.stdout)
    suite(result)

    total = result.testsRun
    failed = len(result.failures) + len(result.errors)
    passed = total - failed
    summary_color = GREEN if failed == 0 else RED
    print(colorize(f"\nResults: {passed}/{total} passed", summary_color))

    return result.wasSuccessful()


if __name__ == "__main__":
    sys.exit(0 if run() else 1)