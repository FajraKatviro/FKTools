
include(../FKUtils/fktools/fktoolsfolder.pri)

mac{
    target.path = "$$FK_TOOLS_FOLDER"
    export(target.path)
}

win32{
    target.path = "$$FK_TOOLS_FOLDER"
    #target.extra = windeployqt --no-translations --qmldir "$$_PRO_FILE_PWD_" "$$DESTDIR"
    export(target.path)
    #export(target.extra)
}

mac|win32{
    INSTALLS += target
    export(INSTALLS)
}

INCLUDEPATH += $$PWD/../FKUtils/sharedHeaders
