glslangValidator.exe -V shader.vert
glslangValidator.exe -V shader.frag
glslangValidator.exe -V DefaultShader.vert -IUtils/ -o DefaultShaderVert.spv
glslangValidator.exe -V DefaultShader.frag -IUtils/ -o DefaultShaderFrag.spv
cmd
