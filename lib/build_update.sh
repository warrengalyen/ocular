#!/bin/bash

# Define the file path
FILE="version.h"

# Read the current VERSION_BUILD
CURRENT_BUILD=$(grep '#define VERSION_BUILD' "$FILE" | awk '{print $3}' | tr -d '"')

# Increment the VERSION_BUILD
NEW_BUILD=$((CURRENT_BUILD + 1))

# Update the VERSION_BUILD in the file
sed -i "s/#define VERSION_BUILD \"$CURRENT_BUILD\"/#define VERSION_BUILD \"$NEW_BUILD\"/" "$FILE"

echo "VERSION_BUILD updated to $NEW_BUILD"