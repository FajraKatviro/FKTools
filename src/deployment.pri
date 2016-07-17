
include(../FKUtils/fktools/fktoolsfolder.pri)

win32{
    deploy.commands = windeployqt --no-translations --qmldir "$$_PRO_FILE_PWD_" "$$FK_TOOLS_FOLDER/$${TARGET}.exe"
    QMAKE_EXTRA_TARGETS += deploy
}

!ios:!android{
    target.path = "$$FK_TOOLS_FOLDER"
    export(target.path)
    INSTALLS += target
    export(INSTALLS)
}

INCLUDEPATH += $$PWD/../FKUtils/sharedHeaders
