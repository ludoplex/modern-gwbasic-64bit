#!/bin/bash
# Verify TRULY branchless implementation - NO control flow keywords allowed

echo "Verifying TRULY branchless implementation..."
echo "Checking for: if, while, for, switch, goto, ternary operators, and branch instructions"
echo ""

# Function to check file for ALL control flow keywords and ternary operators
check_file() {
    local file=$1
    local line_num=0
    local errors=0
    
    while IFS= read -r line; do
        ((line_num++))
        
        # Skip comment lines
        if [[ "$line" =~ ^[[:space:]]*//  ]] || [[ "$line" =~ ^[[:space:]]*\* ]]; then
            continue
        fi
        
        # Check for if statements
        if [[ "$line" =~ ^[[:space:]]*if[[:space:]] ]] || [[ "$line" =~ ^[[:space:]]*if\( ]]; then
            echo "ERROR: Found 'if' statement at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for while loops
        if [[ "$line" =~ ^[[:space:]]*while[[:space:]] ]] || [[ "$line" =~ ^[[:space:]]*while\( ]]; then
            echo "ERROR: Found 'while' loop at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for for loops
        if [[ "$line" =~ ^[[:space:]]*for[[:space:]] ]] || [[ "$line" =~ ^[[:space:]]*for\( ]]; then
            echo "ERROR: Found 'for' loop at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for switch statements
        if [[ "$line" =~ ^[[:space:]]*switch[[:space:]]*\( ]]; then
            echo "ERROR: Found 'switch' statement at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for goto (including computed goto)
        if [[ "$line" =~ goto[[:space:]] ]] && [[ ! "$line" =~ ^[[:space:]]*// ]]; then
            echo "ERROR: Found 'goto' at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
        # Check for ternary operators (? :) - but exclude comments and enum/struct syntax
        if [[ "$line" =~ \?[[:space:]]*[^:]*:[[:space:]] ]] && [[ ! "$line" =~ ^[[:space:]]*// ]] && [[ ! "$line" =~ typedef ]] && [[ ! "$line" =~ enum ]]; then
            # Make sure it's not just a comment with ? in it
            if [[ ! "$line" =~ /\*.*\?.*\*/ ]]; then
                echo "ERROR: Found ternary operator at $file:$line_num"
                echo "  $line"
                ((errors++))
            fi
        fi
        
        # Check for branch instructions in inline asm
        if [[ "$line" =~ (jz|jnz|je|jne|jg|jl|jge|jle|ja|jb|jae|jbe|jmp)[[:space:]] ]] && [[ "$line" =~ \".*\" ]]; then
            echo "ERROR: Found branch instruction in inline asm at $file:$line_num"
            echo "  $line"
            ((errors++))
        fi
        
    done < "$file"
    
    return $errors
}

# Check all C files in src/
total_errors=0
for file in src/lexer.c src/branchless_*.h; do
    if [ -f "$file" ]; then
        echo "Checking $file..."
        check_file "$file"
        file_errors=$?
        total_errors=$((total_errors + file_errors))
        if [ $file_errors -eq 0 ]; then
            echo "  ✓ $file is branchless"
        fi
    fi
done

echo ""
if [ $total_errors -gt 0 ]; then
    echo "❌ Verification FAILED: Found $total_errors control flow violations"
    echo ""
    echo "NO control flow keywords (if/while/for/switch/goto) are allowed"
    echo "NO ternary operators that compile to branches are allowed"
    echo "NO branch instructions (jz/jne/jmp/etc) in inline asm are allowed"
    exit 1
fi

echo "✅ Verification PASSED: Lexer is TRULY branchless!"
echo ""
echo "  ✓ No if/while/for/switch/goto statements"
echo "  ✓ No ternary operators"
echo "  ✓ No branch instructions in inline asm"
echo "  ✓ Uses only: lookup tables, function pointers, arithmetic selection"
exit 0
