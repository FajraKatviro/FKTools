
!defined(ART_FOLDER,var):ART_FOLDER = $$PWD/../art
!defined(ART_BUILD_FOLDER,var):ART_BUILD_FOLDER = $$PWD/../resourceBuild

ART_DEPLOY_FOLDER =

win32{
    ART_DEPLOY_FOLDER = "$$DESTDIR"
}

mac{
    artPackages.files += $$ART_BUILD_FOLDER/bin/*
    artPackages.path = Contents/Resources
    QMAKE_BUNDLE_DATA += artPackages
}

imagesets.commands = "$$PWD/bin/PackageManager" "$$ART_FOLDER" --dir $$escape_expand(\n\t) \
                       "$$PWD/bin/PackageGenerator"  "$$ART_FOLDER" "$$ART_BUILD_FOLDER" $$ART_DEPLOY_FOLDER --clean --add --qrc --rcc --dir $$escape_expand(\n\t)
#Note: missing no quotes around ART_DEPLOY_FOLDER is not a bug

first.depends += qmake

QMAKE_EXTRA_TARGETS += first imagesets
