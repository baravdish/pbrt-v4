#!/bin/bash

# Default values
USE_TEV=false
USE_GPU=false
SPP=64
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
        --use-gpu)
        USE_GPU=true
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
    echo "Usage: $0 --scene <scene_file> --outfile <output_file> [--spp <samples_per_pixel>] [--use-tev] [--use-gpu]"
    exit 1
fi

# Update the .pbrt file
# awk -v spp="$SPP" -v output="$OUTPUT" '
#     /Film "rgb"/ {in_film = 1}
#     in_film && /^\s*"string filename"/ {
#         $0 = "    \"string filename\" [ \"" output "\" ]"
#         in_film = 0
#     }
#     /^\s*"integer pixelsamples"/ {
#         $0 = "    \"integer pixelsamples\" [ " spp " ]"
#     }
#     {print}
# ' "$SCENE" > "${SCENE}.tmp" && mv "${SCENE}.tmp" "$SCENE"

# # Construct and run the rendering command
# if $USE_TEV; then
#     pbrt --display-server localhost:14158 --gpu "$SCENE"
# else
#     pbrt --gpu "$SCENE"
# fi

# Construct and run the rendering command
RENDER_CMD="pbrt --spp $SPP --outfile $OUTPUT"
if $USE_GPU; then
    RENDER_CMD+=" --gpu"
fi
if $USE_TEV; then
    RENDER_CMD+=" --display-server localhost:14158"
fi
RENDER_CMD+=" \"$SCENE\""

# Execute the rendering command
eval "$RENDER_CMD"