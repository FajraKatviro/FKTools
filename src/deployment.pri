
mac{
    target.path = "/$(HOME)/Applications/FKTools"
    export(target.path)
}

win32{
    target.path = "$(APPDATA)/FKTools"
    #target.extra = windeployqt --no-translations --qmldir "$$_PRO_FILE_PWD_" "$$DESTDIR"
    export(target.path)
    #export(target.extra)
}

mac|win32{
    INSTALLS += target
    export(INSTALLS)
}


