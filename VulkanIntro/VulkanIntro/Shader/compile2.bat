for %%f in (*.vert *.tesc *.tese *.geom *.frag *.comp) do %VK_SDK_PATH%\Bin\glslangValidator.exe -V -o %%f.spv %%f
pause