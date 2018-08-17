@echo off
setlocal enabledelayedexpansion

if %1.==. goto noArg
if %2.==. goto noArg

@echo LANGUAGE 0, 0 > %1
@echo #pragma code_page(1252) >> %1
@echo. >> %1
@echo // App Resources >> %1

@echo // App Resources > %2
@echo void SetResources() >> %2
@echo { >> %2

rem ### set the start ID for the resources
set id=10000

rem ### set webappDir to the name of the current directory
for %%f in (%cd%) do set webappDir=%%~nxf

rem ### not used anymore when resources are loaded through the app:// protocol
rem ### Chromium lowercases the first part of the path ("domain")
rem set webappDir=!webappDir:A=a!
rem set webappDir=!webappDir:B=b!
rem set webappDir=!webappDir:C=c!
rem set webappDir=!webappDir:D=d!
rem set webappDir=!webappDir:E=e!
rem set webappDir=!webappDir:F=f!
rem set webappDir=!webappDir:G=g!
rem set webappDir=!webappDir:H=h!
rem set webappDir=!webappDir:I=i!
rem set webappDir=!webappDir:J=j!
rem set webappDir=!webappDir:K=k!
rem set webappDir=!webappDir:L=l!
rem set webappDir=!webappDir:M=m!
rem set webappDir=!webappDir:N=n!
rem set webappDir=!webappDir:O=o!
rem set webappDir=!webappDir:P=p!
rem set webappDir=!webappDir:Q=q!
rem set webappDir=!webappDir:R=r!
rem set webappDir=!webappDir:S=s!
rem set webappDir=!webappDir:T=t!
rem set webappDir=!webappDir:U=u!
rem set webappDir=!webappDir:V=v!
rem set webappDir=!webappDir:W=w!
rem set webappDir=!webappDir:X=x!
rem set webappDir=!webappDir:Y=y!
rem set webappDir=!webappDir:Z=z!

for /R %%f in (*) do (
	set file=%%f

	rem ### Generate the resource file entry of the form
	rem ### "<ID> BINARY <filename>"
	rem ### (We have to escape the backslashes.)
	@echo !id! 256 "!file:\=\\!" >> %1

	rem ### Generate the SetResourceID calls.
	rem ### The resource names have to be relative to the root (we use the current directory, %cd%)
	rem ### and the backslashes have to be replaced by slashes.
	set resname=!file:%cd%\=%webappDir%/!
	@echo Zephyros::SetResourceID(TEXT("!resname:\=/!"^), !id!^); >> %2

	rem ### Increment the counter.
	set /a id=id+1
)

@echo } >> %2

goto done
:noArg
	@echo Syntax: generate_resources.bat "resource-filename" "set-resource-id-include-filename"
:done