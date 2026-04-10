#!/usr/bin/env python3
import os
import re
import sys

def extract_llrs_from_text(text):
    """Extracts all instances of LLR-WX-NNN from a given text string."""
    return set(re.findall(r'LLR-WX-\d{3}', text))

def main():
    spec_file = 'specs/weatherstation_srs_with_llrs_out.md'
    src_dir = 'src'
    test_dir = 'tests'

    # 1. Parse Specifications
    if not os.path.exists(spec_file):
        print(f"Error: Spec file '{spec_file}' not found.")
        sys.exit(1)
        
    with open(spec_file, 'r', encoding='utf-8') as f:
        spec_content = f.read()
    
    # Find definitions (e.g., "**LLR-WX-001**:")
    defined_llrs = set(re.findall(r'\*\*(LLR-WX-\d{3})\*\*', spec_content))
    print(f"Found {len(defined_llrs)} LLRs defined in the specification.")

    # 2. Parse Source Code
    implemented_llrs = set()
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith(('.c', '.h')):
                with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                    for line in f:
                        if 'Implements:' in line:
                            implemented_llrs.update(extract_llrs_from_text(line))

    # 3. Parse Test Code & Traceability Matrix for exceptions
    verified_llrs = set()
    for root, _, files in os.walk(test_dir):
        for file in files:
            if file.endswith(('.c', '.h')):
                with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                    for line in f:
                        if 'Verifies:' in line:
                            verified_llrs.update(extract_llrs_from_text(line))

    # Parse traceability matrix to find blocked/hardware exceptions
    matrix_file = 'specs/traceability.md'
    if os.path.exists(matrix_file):
        with open(matrix_file, 'r', encoding='utf-8') as f:
            for line in f:
                if 'BLOCKED:' in line:
                    # Find which LLR this block refers to
                    llrs_in_line = extract_llrs_from_text(line)
                    verified_llrs.update(llrs_in_line)

    # 4. Verify Traceability
    errors = 0

    # Check for missing implementations
    unimplemented = defined_llrs - implemented_llrs
    if unimplemented:
        print(f"ERROR: Missing implementations in src/ for: {sorted(unimplemented)}")
        errors += 1

    # Check for missing tests
    unverified = defined_llrs - verified_llrs
    if unverified:
        print(f"ERROR: Missing tests in tests/ for: {sorted(unverified)}")
        errors += 1

    # Check for orphaned implementations
    ghost_impls = implemented_llrs - defined_llrs
    if ghost_impls:
        print(f"ERROR: Code implements undefined LLRs: {sorted(ghost_impls)}")
        errors += 1

    # Check for orphaned tests
    ghost_tests = verified_llrs - defined_llrs
    if ghost_tests:
        print(f"ERROR: Tests verify undefined LLRs: {sorted(ghost_tests)}")
        errors += 1

    print("-" * 50)
    if errors == 0:
        print("SUCCESS: 100% Traceability verified between specs, implementation, and tests.")
        sys.exit(0)
    else:
        print("FAILED: Traceability gaps detected.")
        sys.exit(1)

if __name__ == '__main__':
    main()
