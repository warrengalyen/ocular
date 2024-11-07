#!/bin/bash

# Copyright (c) 2024 Warren Galyen
#
# This script generates a C# DLL import definitions from a C header file.
# Created for use with the Ocular library (https://github.com/warrengalyen/ocular)
# but may be useful for other projects that need to interface with a C DLL.
#
# Usage: ./generate_csharp_def.sh
#
# This script requires GNU awk (gawk) to be installed.

# Input and output files
HEADER_FILE="ocular.h"
OUTPUT_FILE="../demo_ui/Interop-test.cs"
TEMP_FILE="temp_functions.txt"
TYPES_FILE="temp_types.txt"

progress_bar() {
    local current=$1
    local total=$2
    local width=50
    local percentage=$((current * 100 / total))
    local completed=$((width * current / total))
    local remaining=$((width - completed))
    local bar=$(printf "%${completed}s" | tr ' ' '=')$(printf "%${remaining}s" | tr ' ' ' ')
    printf "\rGenerating imports: [%s] %d%%" "$bar" "$percentage"
}

echo "Extracting definitions from $HEADER_FILE"

# Extract structs and enums with their names
awk '
BEGIN { in_struct = 0; in_enum = 0; }

# Store type definition and name together
/typedef struct|typedef enum/ {
    if ($0 ~ /typedef struct/) {
        type = "STRUCT"
    } else {
        type = "ENUM"
    }
    
    # Collect the entire definition
    definition = $0
    while (getline line) {
        definition = definition "\n" line
        if (line ~ /}.*;/) break
    }
    
    # Extract the name
    match(definition, /}[[:space:]]*([[:alnum:]_]+)/, arr)
    name = arr[1]
    
    # Output in the desired format
    print "TYPE:" name
    print type "_START"
    print "{"
    
    # Split the definition and print the content
    split(definition, lines, "\n")
    for (i in lines) {
        if (lines[i] !~ /typedef/ && lines[i] !~ /}.*[A-Za-z]/) {
            gsub(/^[[:space:]]+/, "", lines[i])  # Remove leading whitespace
            if (lines[i] != "" && lines[i] !~ /^\/\//) {
                print lines[i]
            }
        }
    }
    print "}"
    print ""
}' "$HEADER_FILE" > "$TYPES_FILE"

# Extract complete function declarations
awk '/DLIB_EXPORT/{
    printf "%s", $0
    if ($0 !~ /;/)
        inside=1
    else
        printf "\n"
    next
}
inside {
    printf " %s", $0
    if ($0 ~ /;/) {
        inside=0
        printf "\n"
    }
}' "$HEADER_FILE" | \
    sed 's/DLIB_EXPORT //' | \
    sed 's/const //' > "$TEMP_FILE"

total_functions=$(wc -l < "$TEMP_FILE")
count=0

# Create the C# file header
cat > "$OUTPUT_FILE" << 'EOF'
using System.Runtime.InteropServices;

namespace OcularDemo
{
    internal static class OcularInterop
    {
EOF

# Process structs and enums
typename=""
while IFS= read -r line; do
    if [[ $line == "TYPE:"* ]]; then
        # Store the type name
        typename=${line#TYPE:}
        continue
    fi
    
    if [[ $line == "STRUCT_START" ]]; then
        echo "        [StructLayout(LayoutKind.Sequential)]" >> "$OUTPUT_FILE"
        echo "        public struct $typename" >> "$OUTPUT_FILE"
    elif [[ $line == "ENUM_START" ]]; then
        echo "        public enum $typename" >> "$OUTPUT_FILE"
    elif [[ $line == "{"* || $line == "}"* ]]; then
        echo "        $line" >> "$OUTPUT_FILE"
        if [[ $line == "}" ]]; then
            echo "" >> "$OUTPUT_FILE"
        fi
    else
        # Convert C types to C# types
        csharp_line=$(echo "$line" | \
            sed 's/unsigned char\*/byte\*/g' | \
            sed 's/unsigned char/byte/g' | \
            sed 's/const //g' | \
            sed 's/int\*/int*/g' | \
            sed 's/float\*/float*/g' | \
            sed 's/void\*/void*/g' | \
            sed 's/bool/bool/g' | \
            sed 's/  \+/ /g' | \
            sed 's/^ *//' | \
            sed 's/;$/;/')

        if [[ -n "$csharp_line" && ! $csharp_line =~ ^[[:space:]]*$ ]]; then
            echo "            $csharp_line" >> "$OUTPUT_FILE"
        fi
    fi
done < "$TYPES_FILE"

# Add memory copy function
cat >> "$OUTPUT_FILE" << 'EOF'
        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = true)]
        public static extern unsafe void CopyMemory(void* Dest, void* Src, int Length);

EOF

# Process each function
while IFS= read -r line; do
    # Extract function name
    func_name=$(echo "$line" | awk '{print $2}' | cut -d'(' -f1)
    
    # Convert C types to C# types and rename Input/Output to Src/Dest
    csharp_line=$(echo "$line" | \
        sed 's/unsigned char\*/byte\*/g' | \
        sed 's/unsigned char/byte/g' | \
        sed 's/const //g' | \
        sed 's/int\*/int*/g' | \
        sed 's/float\*/float*/g' | \
        sed 's/void\*/void*/g' | \
        sed 's/OC_STATUS/int/g' | \
        sed 's/\bInput\b/Src/g' | \
        sed 's/\bOutput\b/Dest/g' | \
        sed 's/\binput\b/Src/g' | \
        sed 's/\boutput\b/Dest/g' | \
        sed 's/  \+/ /g' | \
        sed 's/^ *//')  # Remove leading spaces
    
    # Generate C# DllImport attribute and function declaration
    echo "        [DllImport(\"ocular.dll\", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]" >> "$OUTPUT_FILE"
    echo "        public static extern unsafe $csharp_line" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"

    ((count++))
    progress_bar $count $total_functions
done < "$TEMP_FILE"

# Close the namespace and class
echo "    }" >> "$OUTPUT_FILE"
echo "}" >> "$OUTPUT_FILE"

# Clean up
rm "$TEMP_FILE"
rm "$TYPES_FILE"

echo -e "\n$count C# DLL import definitions have been generated in $OUTPUT_FILE"