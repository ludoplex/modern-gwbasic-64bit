#!/bin/bash
# Verify branchless implementation in AMD64 and AArch64 code paths

echo "Verifying branchless implementation (AMD64/AArch64 code paths)..."

# Function to check file for if/switch outside #else blocks
check_file() {
    local file=$1
    local in_else=0
    local line_num=0
    local errors=0
    
    while IFS= read -r line; do
        ((line_num++))
        
        # Track #else blocks
        if [[ "$line" =~ ^#else ]]; then
            in_else=1
            continue
        fi
        if [[ "$line" =~ ^#endif ]]; then
            in_else=0
            continue
        fi
        
        # Skip if we're in #else block (portable fallback)
        if [ $in_else -eq 1 ]; then
            continue
        fi
        
        # Check for if statements
        if [[ "$line" =~ ^[[:space:]]*if[[:space:]] ]] || [[ "$line" =~ ^[[:space:]]*if\( ]]; then
            echo "ERROR: Found 'if' statement at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for switch statements
        if [[ "$line" =~ ^[[:space:]]*switch[[:space:]]*\( ]]; then
            echo "ERROR: Found 'switch' statement at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
    done < "$file"
    
    return $errors
}

# Check all C files
total_errors=0
for file in src/*.c src/*.h; do
    if [ -f "$file" ]; then
        check_file "$file"
        total_errors=$((total_errors + $?))
    fi
done

if [ $total_errors -gt 0 ]; then
    echo ""
    echo "❌ Verification FAILED: Found $total_errors if/switch statements in AMD64/AArch64 code paths"
    echo ""
    echo "Note: if statements in #else blocks (portable fallback) are allowed"
    exit 1
fi

echo ""
echo "✓ Verification PASSED: AMD64 and AArch64 code paths are branchless!"
echo "  (Portable fallback in #else blocks may use if statements for non-Cosmopolitan builds)"
exit 0
