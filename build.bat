@echo off
echo 正在编译CNC仿真软件...

REM 设置Visual Studio环境
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1
)
if %ERRORLEVEL% neq 0 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" >nul 2>&1
)
if %ERRORLEVEL% neq 0 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1
)

REM 编译项目
echo 开始编译...
msbuild CNCSimulator.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal

if %ERRORLEVEL%==0 (
    echo.
    echo ✅ 编译成功！
    echo 📁 可执行文件位置: x64\Debug\CNCSimulator.exe
    echo.
    echo 🚀 现在可以运行程序测试修复效果：
    echo    - 图形显示的(0,0)点应该在屏幕中心
    echo    - G代码显示应该支持中文字符
    echo.
    echo 📝 测试文件：
    echo    - simple_test.nc (英文注释)
    echo    - test_red_stairs.nc (中文注释，如果仍有乱码请使用simple_test.nc)
) else (
    echo.
    echo ❌ 编译失败，错误代码: %ERRORLEVEL%
    echo 请检查Visual Studio是否正确安装
)

pause
