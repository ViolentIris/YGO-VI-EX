@echo off     
@title 啊，你好啊，欢迎打开程序自带的下载器
@echo 第一次使用下载器时，请务必先注册下载器！
@echo 微博置顶的老喵师牌YGO-VI-EX下载器跟这个是不一样的！不要混在一起用！谢谢支持！谢谢合作！
@echo 再次说明，第一次使用下载器前请务必先注册下载器！
@echo 并且，下载完更新之后！是把打开的压缩包里面出现的唯一的那个文件夹里面的家伙们全部拿出来！同名文件覆盖到YGO-VI-EX主目录！
@echo 程序自带YGO-VI-EX下载器现已升级，对于WIN10系统无需进行手工注册即可实现注册！
@echo 在注册下载器时，第一次下载器会重启，重启后需按0再进行注册步骤即可完成注册！
@echo 注册下载器出现诸如“拒绝访问”等BUG的选手，请及时查看注意事项，进行手工注册！
@echo 具体的内容在下面！
@color 0e

:开始
@echo.
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆
echo ★☆ ★☆ ★☆ ★☆ ★☆★☆★☆ ★☆ ★☆ ★☆ ★☆  
echo ★☆ ★☆     0.注册下载器                 ★☆ ★☆
echo ★☆ ★☆     1.下载YGO-VI-EX完整版        ★☆ ★☆
echo ★☆ ★☆     2.下载YGO-VI-EX更新          ★☆ ★☆
echo ★☆ ★☆     3.娱乐力量全开               ★☆ ★☆
echo ★☆ ★☆     4.恋恋版Koishi主程序         ★☆ ★☆
echo ★☆ ★☆     5.萌卡社区                   ★☆ ★☆
echo ★☆ ★☆     6.老喵师的爱FA电             ★☆ ★☆
echo ★☆ ★☆     7.查看动漫色图               ★☆ ★☆
echo ★☆ ★☆     8.查看注意事项               ★☆ ★☆
echo ★☆ ★☆     9.蛤蟆池联机平台下载         ★☆ ★☆
echo ★☆ ★☆     10.美工补充包下载            ★☆ ★☆
echo ★☆ ★☆     11.YGO皮肤下载               ★☆ ★☆
echo ★☆ ★☆     12.喵预组卡组下载            ★☆ ★☆
echo ★☆ ★☆     13.退出下载器                ★☆ ★☆
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
if %num%==9 (goto 9)
if %num%==10 (goto 10)
if %num%==11 (goto 11)
if %num%==12 (goto 12)
if %num%==13 (exit)  

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
ren "wget-64.exe" "wget.exe"
copy wget.exe c:\\Windows\System32
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
copy wget.exe c:\\Windows\SysWOW64
ren "wget.exe" "wget-64.exe"
pause
goto 开始

:32
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
ren "wget-32.exe" "wget.exe"
copy wget.exe c:\\Windows\System32
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
copy wget.exe c:\\Windows\SysWOW64
ren "wget.exe" "wget-32.exe"
pause
goto 开始

:1
wget -t 0 -c https://gitee.com/vi1911/YGO-VI-EX-Full/repository/archive/master.zip
ren "master.zip" "YGO-VI-EX完整版.zip"
FOR %%F IN (YGO-VI-EX完整版.zip) DO START %%F
pause
goto 开始

 
:2
wget -t 0 -c https://gitee.com/vi1911/YGO-VI-EX-Updates/repository/archive/master.zip
ren "master.zip" "YGO-VI-EX更新.zip"
FOR %%F IN (YGO-VI-EX更新.zip) DO START %%F
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
exit

:8
start "" ".\download\注意事项.txt"
pause
goto 开始
 
:9
start http://www.downza.cn/soft/27231.html
pause
goto 开始

:10
wget -t 0 -c https://gitee.com/vi1911/textures/repository/archive/master.zip
ren "master.zip" "美工补充包.zip"
FOR %%F IN (美工补充包.zip) DO START %%F
pause
goto 开始

 
:11
wget -t 0 -c https://gitee.com/vi1911/skin/repository/archive/master.zip
ren "master.zip" "YGO皮肤（压缩包内文件夹解压到YGO-VI-EX主目录）.zip"
FOR %%F IN (YGO皮肤（压缩包内文件夹解压到YGO-VI-EX主目录）.zip) DO START %%F
pause
goto 开始

:12
wget -t 0 -c https://gitee.com/vi1911/deck/repository/archive/master.zip
ren "master.zip" "喵预组卡组.zip"
FOR %%F IN (喵预组卡组.zip) DO START %%F
pause
goto 开始

:13
exit

@pause   
@echo on