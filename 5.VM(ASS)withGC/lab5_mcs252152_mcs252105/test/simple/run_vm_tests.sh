#!/usr/bin/env bash
set -u
set -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ASM_BIN="$ROOT_DIR/assembler"
VM_BIN="$ROOT_DIR/bvm"
TEST_DIR="$ROOT_DIR/test/simple"
ROOT_TEST_DIR="$ROOT_DIR/test"

if [[ ! -x "$ASM_BIN" || ! -x "$VM_BIN" ]]; then
    echo "error: missing binaries; run 'make' first."
    exit 1
fi

tmp_dir="$(mktemp -d "$ROOT_DIR/test/simple/tmp.XXXXXX")"
trap 'rm -rf "$tmp_dir"' EXIT

fail=0

pass() {
    echo "PASS: $1"
}

fail_case() {
    echo "FAIL: $1"
    fail=1
}

# Test 1: simple program assembles and dumps expected bytes.
simple_bin="$tmp_dir/simple.byc"
if ! "$ASM_BIN" "$TEST_DIR/test.asm" "$simple_bin" >"$tmp_dir/asm_simple.out" 2>&1; then
    fail_case "assemble simple program"
else
    if ! "$VM_BIN" "$simple_bin" >"$tmp_dir/vm_simple.out" 2>"$tmp_dir/vm_simple.err"; then
        fail_case "vm simple program exit"
    elif ! grep -q "01 0A 00 00 00 FF" "$tmp_dir/vm_simple.out"; then
        fail_case "vm simple program output"
    else
        pass "simple program"
    fi
fi

# Test 2: missing HALT should be rejected by VM validation.
nohalt_bin="$tmp_dir/nohalt.byc"
if ! "$ASM_BIN" "$ROOT_TEST_DIR/nohalt.asm" "$nohalt_bin" >/dev/null 2>&1; then
    fail_case "assemble nohalt program"
else
    if "$VM_BIN" "$nohalt_bin" >/dev/null 2>"$tmp_dir/nohalt.err"; then
        fail_case "nohalt should fail"
    elif ! grep -q "no HALT" "$tmp_dir/nohalt.err"; then
        fail_case "nohalt error message"
    else
        pass "nohalt program rejected"
    fi
fi

# Test 3: invalid opcode should be rejected.
printf '\x99' > "$tmp_dir/invalid.byc"
if "$VM_BIN" "$tmp_dir/invalid.byc" >/dev/null 2>"$tmp_dir/invalid.err"; then
    fail_case "invalid opcode should fail"
elif ! grep -q "invalid opcode" "$tmp_dir/invalid.err"; then
    fail_case "invalid opcode error message"
else
    pass "invalid opcode rejected"
fi

# Test 4: stack program runs without errors.
stack_ok_bin="$tmp_dir/stack_ok.byc"
if ! "$ASM_BIN" "$TEST_DIR/stack_ok.asm" "$stack_ok_bin" >/dev/null 2>&1; then
    fail_case "assemble stack_ok program"
else
    if ! "$VM_BIN" "$stack_ok_bin" >/dev/null 2>"$tmp_dir/stack_ok.err"; then
        fail_case "stack_ok should run"
    else
        pass "stack_ok program"
    fi
fi

# Test 5: stack underflow is trapped.
stack_underflow_bin="$tmp_dir/stack_underflow.byc"
if ! "$ASM_BIN" "$TEST_DIR/stack_underflow.asm" "$stack_underflow_bin" >/dev/null 2>&1; then
    fail_case "assemble stack_underflow program"
else
    if "$VM_BIN" "$stack_underflow_bin" >/dev/null 2>"$tmp_dir/stack_underflow.err"; then
        fail_case "stack_underflow should fail"
    elif ! grep -q "stack underflow" "$tmp_dir/stack_underflow.err"; then
        fail_case "stack_underflow error message"
    else
        pass "stack_underflow trapped"
    fi
fi

# Test 6: arithmetic program runs without errors.
arith_bin="$tmp_dir/arith.byc"
if ! "$ASM_BIN" "$TEST_DIR/arith.asm" "$arith_bin" >/dev/null 2>&1; then
    fail_case "assemble arith program"
else
    if ! "$VM_BIN" "$arith_bin" >/dev/null 2>"$tmp_dir/arith.err"; then
        fail_case "arith should run"
    else
        pass "arith program"
    fi
fi

# Test 7: division by zero is trapped.
div_zero_bin="$tmp_dir/div_zero.byc"
if ! "$ASM_BIN" "$TEST_DIR/div_zero.asm" "$div_zero_bin" >/dev/null 2>&1; then
    fail_case "assemble div_zero program"
else
    if "$VM_BIN" "$div_zero_bin" >/dev/null 2>"$tmp_dir/div_zero.err"; then
        fail_case "div_zero should fail"
    elif ! grep -q "division by zero" "$tmp_dir/div_zero.err"; then
        fail_case "div_zero error message"
    else
        pass "div_zero trapped"
    fi
fi

# Test 8: JMP skips over bytes correctly.
jmp_bin="$tmp_dir/jmp.byc"
if ! "$ASM_BIN" "$TEST_DIR/jmp.asm" "$jmp_bin" >/dev/null 2>&1; then
    fail_case "assemble jmp program"
else
    if ! "$VM_BIN" "$jmp_bin" >/dev/null 2>"$tmp_dir/jmp.err"; then
        fail_case "jmp should run"
    else
        pass "jmp program"
    fi
fi

# Test 9: JZ takes jump on zero.
jz_bin="$tmp_dir/jz.byc"
if ! "$ASM_BIN" "$TEST_DIR/jz.asm" "$jz_bin" >/dev/null 2>&1; then
    fail_case "assemble jz program"
else
    if ! "$VM_BIN" "$jz_bin" >/dev/null 2>"$tmp_dir/jz.err"; then
        fail_case "jz should run"
    else
        pass "jz program"
    fi
fi

# Test 10: JNZ takes jump on non-zero.
jnz_bin="$tmp_dir/jnz.byc"
if ! "$ASM_BIN" "$TEST_DIR/jnz.asm" "$jnz_bin" >/dev/null 2>&1; then
    fail_case "assemble jnz program"
else
    if ! "$VM_BIN" "$jnz_bin" >/dev/null 2>"$tmp_dir/jnz.err"; then
        fail_case "jnz should run"
    else
        pass "jnz program"
    fi
fi

# Test 11: STORE/LOAD works with valid index.
mem_ok_bin="$tmp_dir/mem_ok.byc"
if ! "$ASM_BIN" "$TEST_DIR/mem_ok.asm" "$mem_ok_bin" >/dev/null 2>&1; then
    fail_case "assemble mem_ok program"
else
    if ! "$VM_BIN" "$mem_ok_bin" >/dev/null 2>"$tmp_dir/mem_ok.err"; then
        fail_case "mem_ok should run"
    else
        pass "mem_ok program"
    fi
fi

# Test 12: invalid memory index is trapped.
mem_bad_bin="$tmp_dir/mem_bad.byc"
if ! "$ASM_BIN" "$TEST_DIR/mem_bad.asm" "$mem_bad_bin" >/dev/null 2>&1; then
    fail_case "assemble mem_bad program"
else
    if "$VM_BIN" "$mem_bad_bin" >/dev/null 2>"$tmp_dir/mem_bad.err"; then
        fail_case "mem_bad should fail"
    elif ! grep -q "invalid memory index" "$tmp_dir/mem_bad.err"; then
        fail_case "mem_bad error message"
    else
        pass "mem_bad trapped"
    fi
fi

# Test 13: PAIR/LEFT/RIGHT runs without errors.
pair_bin="$tmp_dir/pair_left_right.byc"
if ! "$ASM_BIN" "$TEST_DIR/pair_left_right.asm" "$pair_bin" >/dev/null 2>&1; then
    fail_case "assemble pair_left_right program"
else
    if ! "$VM_BIN" "$pair_bin" >/dev/null 2>"$tmp_dir/pair_left_right.err"; then
        fail_case "pair_left_right should run"
    else
        pass "pair_left_right program"
    fi
fi

# Test 14: CALL/RET runs without errors.
call_bin="$tmp_dir/call.byc"
if ! "$ASM_BIN" "$TEST_DIR/call.asm" "$call_bin" >/dev/null 2>&1; then
    fail_case "assemble call program"
else
    if ! "$VM_BIN" "$call_bin" >/dev/null 2>"$tmp_dir/call.err"; then
        fail_case "call should run"
    else
        pass "call program"
    fi
fi

# Test 15: invalid jump address is trapped.
printf '\x20\xff\xff\xff\x7f\xff' > "$tmp_dir/invalid_jump.byc"
if "$VM_BIN" "$tmp_dir/invalid_jump.byc" >/dev/null 2>"$tmp_dir/invalid_jump.err"; then
    fail_case "invalid jump should fail"
elif ! grep -q "invalid jump address" "$tmp_dir/invalid_jump.err"; then
    fail_case "invalid jump error message"
else
        pass "invalid jump trapped"
fi

# Test 16: non-.byc file is rejected.
printf '\xff' > "$tmp_dir/not_byc.bin"
if "$VM_BIN" "$tmp_dir/not_byc.bin" >/dev/null 2>"$tmp_dir/not_byc.err"; then
    fail_case "non-byc should fail"
elif ! grep -q "expected .byc file" "$tmp_dir/not_byc.err"; then
    fail_case "non-byc error message"
else
        pass "non-byc rejected"
fi

# Test 17: loop with JNZ runs without errors.
loop_bin="$tmp_dir/loop.byc"
if ! "$ASM_BIN" "$ROOT_TEST_DIR/loop.asm" "$loop_bin" >/dev/null 2>&1; then
    fail_case "assemble loop program"
else
    if ! "$VM_BIN" "$loop_bin" >/dev/null 2>"$tmp_dir/loop.err"; then
        fail_case "loop should run"
    else
        pass "loop program"
    fi
fi

# Test 18: Value stack + memory flow runs without errors.
value_flow_bin="$tmp_dir/value_flow.byc"
if ! "$ASM_BIN" "$TEST_DIR/value_flow.asm" "$value_flow_bin" >/dev/null 2>&1; then
    fail_case "assemble value_flow program"
else
    if ! "$VM_BIN" "$value_flow_bin" >/dev/null 2>"$tmp_dir/value_flow.err"; then
        fail_case "value_flow should run"
    else
        pass "value_flow program"
    fi
fi

# Test 19: nested CALL/RET runs without errors.
nested_bin="$tmp_dir/nested_call.byc"
if [[ -f "$ROOT_TEST_DIR/function_call.asm" ]]; then
    if ! "$ASM_BIN" "$ROOT_TEST_DIR/function_call.asm" "$nested_bin" >/dev/null 2>&1; then
        fail_case "assemble nested_call program"
    else
        if ! "$VM_BIN" "$nested_bin" >/dev/null 2>"$tmp_dir/nested_call.err"; then
            fail_case "nested_call should run"
        else
            pass "nested_call program"
        fi
    fi
else
    echo "SKIP: nested_call program (file missing)"
fi

# Test 20: truncated operand is rejected.
printf '\x01' > "$tmp_dir/trunc.byc"
if "$VM_BIN" "$tmp_dir/trunc.byc" >/dev/null 2>"$tmp_dir/trunc.err"; then
    fail_case "truncated should fail"
elif ! grep -q "truncated instruction" "$tmp_dir/trunc.err"; then
    fail_case "truncated error message"
else
    pass "truncated rejected"
fi

if [[ $fail -ne 0 ]]; then
    echo "VM tests failed."
    exit 1
fi

echo "All VM tests passed."
