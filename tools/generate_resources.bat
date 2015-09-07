@echo off
setlocal enabledelayedexpansion

if %1.==. goto noArg
if %2.==. goto noArg

@echo LANGUAGE 0, 0 > %1
@echo #pragma code_page(1252) >> %1
@echo >> %1
@echo // App Resources >> %1

@echo // App Resources > %2

rem ### set the start ID for the resources
set id=10000

for /R %%f in (*) do (
	set file=%%f

	rem ### Generate the resource file entry of the form
	rem ### "<ID> BINARY <filename>"
	rem ### (We have to escape the backslashes.)
	@echo !id! 256 "!file:\=\\!" >> %1

	rem ### Generate the SetResourceID calls.
	rem ### The resource names have to be relative to the root (we use the current directory, %cd%)
	rem ### and the backslashes have to be replaced by slashes.
	set resname=!file:%cd%\=!
	@echo Zephyros::SetResourceID(TEXT("!resname:\=/!"^), !id!^); >> %2

	rem ### Increment the counter.
	set /a id=id+1
)

goto done
:noArg
	@echo Syntax: generate_resources.bat "resource-filename" "set-resource-id-include-filename"
:done