%VULKAN_SDK%\Bin\glslangValidator.exe -V -o shader.vert.spv shader.vert
xcopy /I /Q /Y shader.vert.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y shader.vert.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y shader.vert.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y shader.vert.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o shader.frag.spv shader.frag
xcopy /I /Q /Y shader.frag.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y shader.frag.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y shader.frag.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y shader.frag.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o rt_basic.rgen.spv rt_basic.rgen
xcopy /I /Q /Y rt_basic.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y rt_basic.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y rt_basic.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y rt_basic.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

pause