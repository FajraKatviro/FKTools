
!defined(ART_FOLDER,var):ART_FOLDER = $$PWD/../art
!defined(ART_BUILD_FOLDER,var):ART_BUILD_FOLDER = $$PWD/../resourceBuild

artPackageList = $$files($$ART_FOLDER/*)
for(pkg, artPackageList){
    artPackages.files += $$ART_BUILD_FOLDER/bin/$$relative_path($$pkg,$$ART_FOLDER)
}

win32{
    artPackages.path = $$DESTDIR
    artPackages.extra += $$PWD/bin/PackageManager $$ART_FOLDER --dir $$escape_expand(\n\t)
    artPackages.extra += $$PWD/bin/PackageGenerator  $$ART_FOLDER $$ART_BUILD_FOLDER --clean --add --qrc --rcc --dir
    artPackages.CONFIG += no_check_exist directory
    INSTALLS += artPackages
}

mac{
    artPackages.path = Contents/Resources
    QMAKE_BUNDLE_DATA += artPackages
}

