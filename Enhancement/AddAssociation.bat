@echo off

assoc .sp=suanpanmodel
assoc .supan=suanpanmodel

set "program=%~dp0suanPan.exe"

if not exist "%program%" (
	echo suanPan.exe does not exist in current folder
	goto byebye
)

ftype suanpanmodel="%program%" "-f" "%%1"

set "program=%program:\=/%"

set "target=%appdata%\Sublime Text 3\Packages\User"

if exist "%target%" (
	echo {"cmd":["%program%","-f","$file"],"selector":"source.supan","file_patterns":["*.supan","*.sp"]} > "%~dp0suanPan.sublime-build"
	xcopy "%~dp0suanPan.sublime*" "%target%"
)

:byebye