#!/bin/bash

# Default values
USE_TEV=false
SPP=4096
SCENE=""
OUTPUT=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --use-tev)
        USE_TEV=true
        shift
        ;;
        --spp)
        SPP="$2"
        shift
        shift
        ;;
        --scene)
        SCENE="$2"
        shift
        shift
        ;;
        --outfile)
        OUTPUT="$2"
        shift
        shift
        ;;
        *)
        echo "Unknown option: $1"
        exit 1
        ;;
    esac
done

# Check if required arguments are provided
if [ -z "$SCENE" ] || [ -z "$OUTPUT" ]; then
    echo "Usage: $0 --scene <scene_file> --outfile <output_file> [--spp <samples_per_pixel>] [--use-tev]"
    exit 1
fi

# Update the .pbrt file
sed -i "s/\"integer pixelsamples\".*$/\"integer pixelsamples\" [ $SPP ]/" "$SCENE"
sed -i "s|\"string filename\".*$|\"string filename\" [ \"$OUTPUT\" ]|" "$SCENE"

# Construct and run the rendering command
if $USE_TEV; then
    pbrt --display-server localhost:14158 --gpu "$SCENE"
else
    pbrt --gpu "$SCENE"
fi
