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


%VULKAN_SDK%\Bin\glslangValidator.exe -V -o hello_rt.rchit.spv rt_09_first.rchit
xcopy /I /Q /Y hello_rt.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y hello_rt.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o hello_rt.rgen.spv rt_09_first.rgen
xcopy /I /Q /Y hello_rt.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y hello_rt.rgen.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o hello_rt.rmiss.spv rt_09_first.rmiss
xcopy /I /Q /Y hello_rt.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y hello_rt.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y hello_rt.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o hello_rt_scnd.rchit.spv rt_09_secondary.rchit
xcopy /I /Q /Y hello_rt_scnd.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rchit.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

%VULKAN_SDK%\Bin\glslangValidator.exe -V -o hello_rt_scnd.rmiss.spv rt_09_secondary.rmiss
xcopy /I /Q /Y hello_rt_scnd.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Debug_GL46_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Release_GL46_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Debug_Vulkan_x64\shader\
xcopy /I /Q /Y hello_rt_scnd.rmiss.spv ..\..\visual_studio\examples\hello_world\bin\Release_Vulkan_x64\shader\

pause