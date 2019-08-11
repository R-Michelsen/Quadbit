if not exist "../../../../Bin/Release/x64/Resources/Shaders/Compiled" mkdir "../../../../Bin/Release/x64/Resources/Shaders/Compiled"
if not exist "../../../../Bin/Debug/x64/Resources/Shaders/Compiled" mkdir "../../../../Bin/Debug/x64/Resources/Shaders/Compiled"

glslangValidator.exe	 -V water_frag.glsl					-S frag		-o Compiled/water_frag.spv
glslangValidator.exe	 -V water_frag.glsl					-S frag		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/water_frag.spv
glslangValidator.exe	 -V water_frag.glsl					-S frag		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/water_frag.spv
						
glslangValidator.exe	 -V water_vert.glsl					-S vert		-o Compiled/water_vert.spv
glslangValidator.exe	 -V water_vert.glsl					-S vert		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/water_vert.spv
glslangValidator.exe	 -V water_vert.glsl					-S vert		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/water_vert.spv

glslangValidator.exe	 -V precalc_comp.glsl				-S comp		-o Compiled/precalc_comp.spv
glslangValidator.exe	 -V precalc_comp.glsl				-S comp		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/precalc_comp.spv
glslangValidator.exe	 -V precalc_comp.glsl				-S comp		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/precalc_comp.spv

glslangValidator.exe	 -V waveheight_comp.glsl			-S comp		-o Compiled/waveheight_comp.spv
glslangValidator.exe	 -V waveheight_comp.glsl			-S comp		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/waveheight_comp.spv
glslangValidator.exe	 -V waveheight_comp.glsl			-S comp		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/waveheight_comp.spv

glslangValidator.exe	 -V ifft_comp.glsl					-S comp		-o Compiled/ifft_comp.spv
glslangValidator.exe	 -V ifft_comp.glsl					-S comp		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/ifft_comp.spv
glslangValidator.exe	 -V ifft_comp.glsl					-S comp		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/ifft_comp.spv

glslangValidator.exe	 -V displacement_comp.glsl			-S comp		-o Compiled/displacement_comp.spv
glslangValidator.exe	 -V displacement_comp.glsl			-S comp		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/displacement_comp.spv
glslangValidator.exe	 -V displacement_comp.glsl			-S comp		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/displacement_comp.spv

glslangValidator.exe	 -V skybox_frag.glsl					-S frag		-o Compiled/skybox_frag.spv
glslangValidator.exe	 -V skybox_frag.glsl					-S frag		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/skybox_frag.spv
glslangValidator.exe	 -V skybox_frag.glsl					-S frag		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/skybox_frag.spv
						
glslangValidator.exe	 -V skybox_vert.glsl					-S vert		-o Compiled/skybox_vert.spv
glslangValidator.exe	 -V skybox_vert.glsl					-S vert		-o ../../../../Bin/Release/x64/Resources/Shaders/Compiled/skybox_vert.spv
glslangValidator.exe	 -V skybox_vert.glsl					-S vert		-o ../../../../Bin/Debug/x64/Resources/Shaders/Compiled/skybox_vert.spv