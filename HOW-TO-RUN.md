
# How to PBRTv4

- Use ImageMagick tool to convert the output .exr EXR file to .png like this: convert output.exr output.png
- Use the --gpu flag to run on GPU like this: ./pbrt --gpu my-scene.pbrt
- Use --optix flag to run with OptiX accelerated ray tracing like this: ./pbrt --optix my-scene.pbrt
- They can also be combined like this: ./pbrt --gpu --optix my-scene.pbrt

## Without GPU compilation

- Since we use NVIDIA driver 535 (only supports 2 monitors on Ubuntu 24.04) the maximum CUDA capability is 12.2
- Either install CUDA 12.2 (only supports Ubuntu 22.04, not tested for 24.04) or just skip it.
- If you skip it run with this command:

ONLY CPU: cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CUDA_COMPILER="" -DPBRT_OPTIX7_PATH=""

## Do NOT install OpenEXR via apt

- Let PBRTv4 install OpenEXR. Do NOT attempt to install it your own via apt. It will not work.


## Scenes
- https://github.com/mmp/pbrt-v4-scenes
- https://benedikt-bitterli.me/resources/
- https://github.com/shadeops/pbrt-v4-volumes
- https://github.com/shadeops/pbrt-v4-menger
- https://github.com/stig-atle/io_scene_pbrt/tree/Pbrt-v4-support

## Tools 

- There are a lot of tools to export and import .pbrt scene file format from Blender meshes, Houdini and other renderers.

## Interactive with tev

- Download tev AppImage from their Github release
- Start the tev.AppImage program and keep the window open
- Render with PBRT by targeting a port like this: pbrt --display-server localhost:14158 scene-v4.pbrt
- Render with the Bash script on CPU render-cornell.sh: ./render-cornell.sh --scene scene-v4.pbrt --outfile /mnt/b2cloudstorage/Rendering-MC/PBRTv4/renderings/cornell-box-128spp.exr --spp 128 --use-tev
- Render with the Bash script on GPU render-cornell.sh: ./render-cornell.sh --scene scene-v4.pbrt --outfile /mnt/b2cloudstorage/Rendering-MC/PBRTv4/renderings/cornell-box-128spp.exr --spp 128 --use-tev --use-gpu
