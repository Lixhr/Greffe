set -eo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IDAT="${HOME}/.ida-pro-9.2/idat"
SCRIPT="$TESTS_DIR/test.py"

echo "==> Building test binaries..."
make -C "$TESTS_DIR" re

VARIANTS=(
    "aarch64        qemu-aarch64     no"
    "aarch64-be     qemu-aarch64_be  no"
    "arm32          qemu-arm         yes"
    "arm32-be       qemu-armeb       yes"
    "arm32-thumb    qemu-arm         no"
    "arm32-thumb-be qemu-armeb       no"
)

PASS=0
FAIL=0

for entry in "${VARIANTS[@]}"; do
    read -r dir qemu force_arm <<< "$entry"

    binary="$TESTS_DIR/$dir/build/test"
    patched="/tmp/greffe_patched_${dir}"
    logfile="/tmp/greffe_test_${dir}.log"

    echo ""
    echo "=== [$dir] instrumenting with IDA ==="

    ida_sarg="$SCRIPT $patched"
    [ "$force_arm" = "yes" ] && ida_sarg="$ida_sarg --force-arm"

    if ! "$IDAT" -L"$logfile" -P- -A -S"$ida_sarg" "$binary"; then
        echo "FAIL [$dir]: idat exited non-zero"
        cat "$logfile"
        FAIL=$((FAIL + 1))
        continue
    fi

    if [ ! -f "$patched" ]; then
        echo "FAIL [$dir]: patched binary not produced"
        cat "$logfile"
        FAIL=$((FAIL + 1))
        continue
    fi

    echo "=== [$dir] running patched binary under $qemu ==="
    output=$("$qemu" "$patched" 2>&1)
    echo "$output"
    if echo "$output" | grep -q "CRC OK"; then
        echo "PASS [$dir]"
        PASS=$((PASS + 1))
    else
        echo "FAIL [$dir]: 'CRC OK' not found in output"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: ${PASS} passed, ${FAIL} failed"
[ "$FAIL" -eq 0 ]
