
#checkImagesetsStep.commands = bin/PackageManager

#generateImagesetsStep.commands = bin/PackageGenerator
#generateImagesetsStep.depends = checkImagesetsStep

#buildImagesetsStep.commands = rcc
#buildImagesetsStep.depends = generateImagesetsStep

#first.depends += buildImagesetsStep
