Not for QML designer.\
\
The file main.qml, folders "resources" and "shared" should be placed one level top, than the executable.
\
In case of Debugger (e.g. CDB) doesn't work,\
turn on Menu Views > Debugger Global Log\
probably your error is "debugging failed, Win32 error 0n87 "The parameter is incorrect."\
(means "no exe-file find to debug"), just recreate the RUN configuration in Qt\

![x86](https://github.com/roosslan/qml_IFC_exporter/blob/main/exportToWindow.gif?raw=true) \
--
