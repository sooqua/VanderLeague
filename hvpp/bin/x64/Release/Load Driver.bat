SET cwd=%~dp0
sc create hvpp type= kernel binPath= "%cwd%hvppdrv.sys"
sc start hvpp
pause