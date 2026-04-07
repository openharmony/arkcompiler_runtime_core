@echo off
REM Copyright (c) 2026 Huawei Device Co., Ltd.
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..\..") do set "REPO_ROOT=%%~fI"
set "BOOTSTRAP_PY=%REPO_ROOT%\src\arkts_migration_visualizer\deps\bootstrap_impl.py"
call :normalize_optional_path_var BOOTSTRAP_PYTHON
call :normalize_optional_path_var HAPRAY_PYTHON

for %%V in (BOOTSTRAP_PYTHON HAPRAY_PYTHON) do (
  call set "_OPTIONAL_PY=%%%V%%"
  if defined _OPTIONAL_PY (
    call "%_OPTIONAL_PY%" "%BOOTSTRAP_PY%" %*
    exit /b %errorlevel%
  )
)
set "_OPTIONAL_PY="

for %%P in ("%REPO_ROOT%\.py311\Scripts\python.exe" "%REPO_ROOT%\.py311\bin\python") do (
  if exist "%%~fP" (
    call "%%~fP" "%BOOTSTRAP_PY%" %*
    exit /b %errorlevel%
  )
)

where py >nul 2>nul
if not errorlevel 1 (
  py -3 "%BOOTSTRAP_PY%" %*
  exit /b %errorlevel%
)

for %%L in (python python3) do (
  where %%L >nul 2>nul
  if not errorlevel 1 (
    call %%L "%BOOTSTRAP_PY%" %*
    exit /b %errorlevel%
  )
)

echo No suitable Python launcher found. Set BOOTSTRAP_PYTHON or HAPRAY_PYTHON first. 1>&2
exit /b 1

:normalize_optional_path_var
call set "_OPTIONAL_PATH=%%%~1%%"
if not defined _OPTIONAL_PATH exit /b 0
if '%_OPTIONAL_PATH:~0,1%'=='"' if '%_OPTIONAL_PATH:~-1%'=='"' set "_OPTIONAL_PATH=%_OPTIONAL_PATH:~1,-1%"
if not defined _OPTIONAL_PATH (
  set "%~1="
  exit /b 0
)
if not exist "%_OPTIONAL_PATH%" (
  set "%~1="
  exit /b 0
)
set "%~1=%_OPTIONAL_PATH%"
set "_OPTIONAL_PATH="
exit /b 0
