
checkImagesets.commands = $$PWD/bin/PackageManager $$ART_FOLDER --dir
generateImagesets.commands = $$PWD/bin/PackageGenerator  $$ART_FOLDER $$ART_BUILD_FOLDER --clean --add --qrc --rcc --dir

generateImagesets.depends = checkImagesets
imageset.depends = generateImagesets #command line alias
QMAKE_EXTRA_TARGETS += imageset checkImagesets generateImagesets

artPackages.files = $$ART_BUILD_FOLDER/bin/*
artPackages.path = $$DESTDIR
INSTALLS += artPackages

export(INSTALLS)

