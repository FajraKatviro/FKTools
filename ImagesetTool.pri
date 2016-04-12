
checkImagesetsStep.commands = bin/PackageManager $$ART_FOLDER --dir
generateImagesetsStep.commands = bin/PackageGenerator  $$ART_FOLDER $$ART_BUILD_FOLDER --clean --add --qrc --rcc --dir

CONFIG(buildImagePackages){
    generateImagesetsStep.depends = checkImagesetsStep
    first.depends += generateImagesetsStep

    artPackages.files = $$ART_BUILD_FOLDER/bin/*
    artPackages.path = /
    INSTALLS += artPackages
}

