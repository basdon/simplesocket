@ECHO OFF
IF [%~1] EQU [clean] (
	IF EXIST out RD out /S /Q
	MKDIR out
)
SET "PLUGINTYPE=dll"
make build
