SET cwd=%~dp0
sc create HyperLoL binpath="%cwd%HyperLoL.sys" type=kernel
sc start HyperLoL