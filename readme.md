QML designer v18+.\
\
The file main.qml, folders "resources" and "shared" should be placed one level top, than the executable. \
The file moc_predefs.h should be in "debug" and "release" folders. \
\
If error "File Makefile doesn't exist" occurs, that means your command-line interpreter cmd.exe runs and operate in wrong directory. As a temporary solution, you could change Sysroot parameter of your Qt Kit to the ifc_exporter's project directory. \
Check your registry REG_SZ: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Command Processor\Autorun
\
In case of Debugger (e.g. CDB) doesn't work,\
turn on Menu Views > Debugger Global Log\
probably your error is "debugging failed, Win32 error 0n87 "The parameter is incorrect."\
(means "no exe-file find to debug"), just recreate the RUN configuration in Qt

![x86](https://github.com/roosslan/qml_IFC_exporter/blob/trunk/ifc_exporter.gif?raw=true)
--
