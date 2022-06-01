glslangValidator.exe -t -V DefaultShader.vert -IUtils/ -o DefaultShaderVert.spv
glslangValidator.exe -t -V DefaultShader.frag -IUtils/ -o DefaultShaderFrag.spv

glslangValidator.exe -t -V DrawSkyBoxShader.vert -IUtils/ -o DrawSkyBoxShaderVert.spv
glslangValidator.exe -t -V DrawSkyBoxShader.frag -IUtils/ -o DrawSkyBoxShaderFrag.spv

glslangValidator.exe -t -V DefaultTransprantShader.vert -IUtils/ -o DefaultTransparentShaderVert.spv
glslangValidator.exe -t -V DefaultTransprantShader.frag -IUtils/ -o DefaultTransparentShaderFrag.spv
cmd
