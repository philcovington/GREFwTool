TEMPLATE = aux
DESTDIR = ./
OBJECTS_DIR = ./
MOC_DIR = ./
PROJECT_DIR = $$clean_path($$PWD/../)
VERSION = $$fromfile($$PROJECT_DIR/GREFwTool.pro, VERSION)

INSTALLER = GREFwToolWin$$VERSION

INPUT = $$PROJECT_DIR/build/windows-release/release/GREFwTool.exe $$PWD/config/config.xml $$PWD/packages
installer.input = INPUT
installer.output = $$INSTALLER
installer.commands += $(COPY) $$shell_path($$PROJECT_DIR/build/windows-release/release/GREFwTool.exe) $$shell_path($$OUT_PWD/release) &&
installer.commands += $(COPY) $$shell_path($$PROJECT_DIR/documentation/*.*) $$shell_path($$OUT_PWD/release) &&
installer.commands += windeployqt $$shell_path($$OUT_PWD/release) &&
installer.commands += archivegen $$PWD/packages/info.linuxsheeple.grefwtool/data/files.7z $$OUT_PWD/release/* &&
installer.commands += binarycreator --offline-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
installer.CONFIG += target_predeps no_link combine

QMAKE_EXTRA_COMPILERS += installer
