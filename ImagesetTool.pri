
!defined(ART_FOLDER,var):ART_FOLDER = $$PWD/../art
!defined(ART_BUILD_FOLDER,var):ART_BUILD_FOLDER = $$PWD/../resourceBuild

artPackages.files = $$ART_BUILD_FOLDER/bin/*
artPackages.path = $$DESTDIR
artPackages.extra += $$PWD/bin/PackageManager $$ART_FOLDER --dir $$escape_expand(\n\t)
artPackages.extra += $$PWD/bin/PackageGenerator  $$ART_FOLDER $$ART_BUILD_FOLDER --clean --add --qrc --rcc --dir
artPackages.CONFIG += no_check_exist directory
INSTALLS += artPackages

