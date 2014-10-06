@echo off

cd %CD%\tools\project-creator

:label_chapter
set /p chapter=Chapter:
if "%chapter%" == "" goto label_chapter

:label_project
set /p project=Project:
if "%project%" == "" goto label_project

python create_project.py -c "%chapter%" -p "%project%"

pause
