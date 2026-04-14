#!/bin/bash

# Decompression script for large assets
echo "Decompressing large 3D models..."

find assets -name "*.obj.gz" | while read -r file; do
    echo "Decompressing $file..."
    gzip -dkf "$file"
done

echo "Done. All assets are ready."
