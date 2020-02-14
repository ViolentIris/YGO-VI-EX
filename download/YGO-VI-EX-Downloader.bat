@echo off     
@title 欢迎使用老喵师牌YGO-VI-EX下载器
@echo 第一次使用下载器时，请务必先注册下载器！
@echo 系统没装在C盘的大仙，或者用其他系统的大仙，请您另寻高人，或咨询老喵师！
@echo 谢谢支持！谢谢合作！
@echo 今天是：%date%    
@echo 现在是：%time% 
@echo 啊，多么神奇的一天！
@echo 老喵师牌YGO-VI-EX下载器现已升级，对于WIN10系统无需进行手工注册即可实现注册！
@echo 取得管理员权限时，下载器有概率会重启，此时需按0进行注册步骤完成注册！
@echo 注册下载器出现诸如“拒绝访问”等BUG的选手，请及时查看注意事项，进行手工注册！
@color 0e

:开始
@echo.
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆ 
echo ★☆                                            ☆★ 
echo ★☆ ★☆     0.注册下载器                 ★☆ ★☆
echo ★☆ ★☆     1.下载YGO-VI-EX完整版        ★☆ ★☆
echo ★☆ ★☆     2.下载YGO-VI-EX更新          ★☆ ★☆
echo ★☆ ★☆     3.娱乐力量全开               ★☆ ★☆
echo ★☆ ★☆     4.恋恋版Koishi主程序         ★☆ ★☆
echo ★☆ ★☆     5.萌卡社区                   ★☆ ★☆
echo ★☆ ★☆     6.老喵师的爱FA电             ★☆ ★☆
echo ★☆ ★☆     7.查看动漫色图               ★☆ ★☆
echo ★☆ ★☆     8.查看注意事项               ★☆ ★☆
echo ★☆ ★☆     9.退出下载器                 ★☆ ★☆
echo ★☆                                            ☆★
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆
@echo.
set /p num=请输入对应数字，按ENTER确定（直接按ENTER可重复上一操作）：
if %num%==1 (goto 1)
if %num%==2 (goto 2)
if %num%==3 (goto 3)
if %num%==4 (goto 4)
if %num%==5 (goto 5)
if %num%==6 (goto 6)
if %num%==7 (goto 7)
if %num%==8 (goto 8)
if %num%==9 (exit)  

:0

@echo.
echo 请根据您的操作系统类型，选择合适的注册方式哦！
@echo.
echo 您是64位的操作系统，还是32位的操作系统呢？
echo 64.64位操作系统
echo 32.32位操作系统
echo 00.返回上一级
set /p num=请输入对应类型，按ENTER确定（直接按ENTER可重复上一操作）：
if %num%==64 (goto 64)
if %num%==32 (goto 32)
if %num%==00 (goto 开始)

:64
ren "wget-64.exe" "wget.exe"
if exist "%SystemRoot%\System32" path %path%;%windir%\SysNative;%SystemRoot%\System32;%~dp0
bcdedit >nul
if '%errorlevel%' NEQ '0' (goto UACPrompt) else (goto UACAdmin)
:UACPrompt
%1 start "" mshta vbscript:createobject("shell.application").shellexecute("""%~0""","::",,"runas",1)(window.close)&exit
exit /B
:UACAdmin
cd /d "%~dp0"
echo 当前运行路径是：%CD%
echo 已获取管理员权限
if exist "%SystemRoot%\SysWOW64" path %path%;%windir%\SysNative;%SystemRoot%\SysWOW64;%~dp0
bcdedit >nul
if '%errorlevel%' NEQ '0' (goto UACPrompt) else (goto UACAdmin)
:UACPrompt
%1 start "" mshta vbscript:createobject("shell.application").shellexecute("""%~0""","::",,"runas",1)(window.close)&exit
exit /B
:UACAdmin
cd /d "%~dp0"
echo 当前运行路径是：%CD%
echo 已获取管理员权限
copy wget.exe c:\\Windows\System32
copy wget.exe c:\\Windows\SysWOW64
ren "wget.exe" "wget-64.exe"
pause
goto 开始

:32
ren "wget-32.exe" "wget.exe"
if exist "%SystemRoot%\System32" path %path%;%windir%\SysNative;%SystemRoot%\System32;%~dp0
bcdedit >nul
if '%errorlevel%' NEQ '0' (goto UACPrompt) else (goto UACAdmin)
:UACPrompt
%1 start "" mshta vbscript:createobject("shell.application").shellexecute("""%~0""","::",,"runas",1)(window.close)&exit
exit /B
:UACAdmin
cd /d "%~dp0"
echo 当前运行路径是：%CD%
echo 已获取管理员权限
if exist "%SystemRoot%\SysWOW64" path %path%;%windir%\SysNative;%SystemRoot%\SysWOW64;%~dp0
bcdedit >nul
if '%errorlevel%' NEQ '0' (goto UACPrompt) else (goto UACAdmin)
:UACPrompt
%1 start "" mshta vbscript:createobject("shell.application").shellexecute("""%~0""","::",,"runas",1)(window.close)&exit
exit /B
:UACAdmin
cd /d "%~dp0"
echo 当前运行路径是：%CD%
echo 已获取管理员权限
copy wget.exe c:\\Windows\System32
copy wget.exe c:\\Windows\SysWOW64
ren "wget.exe" "wget-32.exe"
pause
goto 开始

:1
wget -t 0 -c https://gitee.com/vi1911/YGO-VI-EX-Full/repository/archive/master.zip
ren "master.zip" "YGO-VI-EX完整版.zip"
pause
goto 开始

 
:2
wget -t 0 -c https://gitee.com/vi1911/YGO-VI-EX-Updates/repository/archive/master.zip
ren "master.zip" "YGO-VI-EX更新.zip"
pause
goto 开始

:3
start http://ygocore.ys168.com/
pause
goto 开始

:4
start http://koishi.222diy.gdn/download.html
pause
goto 开始

:5
start https://ygobbs.com/
pause
goto 开始

:6
start https://afdian.net/@YGO-VI
pause
goto 开始

:7
@echo.
echo 看什么色图！变态！
echo 想的啥呢！满脑子都是色图！
echo 精虫上脑了吧你！
echo 要看色图自己去下啊！满脑子色图的大变态！
echo 哼！自己去撸自己去吧！
echo 精虫上脑的大变态！
pause

:8
start "" ".\download\注意事项.txt"
pause
goto 开始
 
:9
exit

@pause   
@echo on