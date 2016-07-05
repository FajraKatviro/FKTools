
isEmpty(ART_FOLDER){
    error("ART_FOLDER is not set")
}
isEmpty(ART_BUILD_FOLDER){
    error("ART_BUILD_FOLDER is not set")
}

ART_DEPLOY_FOLDER =

win32{
    ART_DEPLOY_FOLDER = "$$DESTDIR"
}

mac{
    !isEmpty(QMAKE_POST_LINK): QMAKE_POST_LINK += ";"
    ios{
        QMAKE_POST_LINK += \
            "cp -r $$ART_BUILD_FOLDER/bin/* $CODESIGNING_FOLDER_PATH/"
    }else{
        QMAKE_POST_LINK += \
            "cp -r $$ART_BUILD_FOLDER/bin/* $$DESTDIR/$${TARGET}.app/Contents/Resources"
    }
}

imagesets.commands =   "$$PWD/bin/PackageManager" "$$ART_FOLDER" --dir $$escape_expand(\n\t) \
                       "$$PWD/bin/PackageGenerator"  "$$ART_FOLDER" "$$ART_BUILD_FOLDER" $$ART_DEPLOY_FOLDER --clean --add --qrc --rcc --dir $$escape_expand(\n\t)
#Note: missing no quotes around ART_DEPLOY_FOLDER is not a bug

first.depends += qmake

QMAKE_EXTRA_TARGETS += first imagesets
