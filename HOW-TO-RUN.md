# Without GPU compilation

- Since we use NVIDIA driver 535 (only supports 2 monitors on Ubuntu 24.04) the maximum CUDA capability is 12.2
- Either install CUDA 12.2 (only supports Ubuntu 22.04, not tested for 24.04) or just skip it.
- If you skip it run with this command:

ONLY CPU: cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CUDA_COMPILER="" -DPBRT_OPTIX7_PATH=""

# Do NOT install OpenEXR via apt

- Let PBRTv4 install OpenEXR. Do NOT attempt to install it your own via apt. It will not work.
