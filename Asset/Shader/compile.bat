glslangValidator.exe -t -V DefaultShader.vert -IUtils/ -o DefaultShaderVert.spv
glslangValidator.exe -t -V DefaultShader.frag -IUtils/ -o DefaultShaderFrag.spv

glslangValidator.exe -t -V DefaultBackgroundShader.vert -IUtils/ -o DefaultBackgroundShaderVert.spv
glslangValidator.exe -t -V DefaultBackgroundShader.frag -IUtils/ -o DefaultBackgroundShaderFrag.spv

glslangValidator.exe -t -V DefaultTransprantShader.vert -IUtils/ -o DefaultTransparentShaderVert.spv
glslangValidator.exe -t -V DefaultTransprantShader.frag -IUtils/ -o DefaultTransparentShaderFrag.spv
cmd
