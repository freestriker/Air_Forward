glslangValidator.exe -V DefaultShader.vert -IUtils/ -o DefaultShaderVert.spv
glslangValidator.exe -V DefaultShader.frag -IUtils/ -o DefaultShaderFrag.spv

glslangValidator.exe -V DrawSkyBoxShader.vert -IUtils/ -o DrawSkyBoxShaderVert.spv
glslangValidator.exe -V DrawSkyBoxShader.frag -IUtils/ -o DrawSkyBoxShaderFrag.spv

glslangValidator.exe -V DefaultTransprantShader.vert -IUtils/ -o DefaultTransparentShaderVert.spv
glslangValidator.exe -V DefaultTransprantShader.frag -IUtils/ -o DefaultTransparentShaderFrag.spv
cmd
