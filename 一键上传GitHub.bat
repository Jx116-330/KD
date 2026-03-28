@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ================================
echo GitHub 一键上传脚本
echo 当前目录：%cd%
echo ================================
echo.

set /p msg=请输入本次提交说明: 

if "%msg%"=="" (
    echo 提交说明不能为空，已取消。
    pause
    exit /b 1
)

echo.
echo [1/4] 检查仓库状态...
git status
if errorlevel 1 (
    echo 当前目录不是 Git 仓库，或 Git 未正确安装。
    pause
    exit /b 1
)

echo.
echo [2/4] 添加改动...
git add .
if errorlevel 1 (
    echo git add 失败。
    pause
    exit /b 1
)

echo.
echo [3/4] 提交改动...
git commit -m "%msg%"
if errorlevel 1 (
    echo.
    echo 可能没有可提交的改动，或者提交失败。
    pause
    exit /b 1
)

echo.
echo [4/4] 上传到 GitHub...
git push
if errorlevel 1 (
    echo git push 失败，请检查网络或登录状态。
    pause
    exit /b 1
)

echo.
echo 上传完成。
pause
