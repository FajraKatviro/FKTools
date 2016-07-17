TEMPLATE = subdirs

SUBDIRS += \
    ImageManager \
    PackageGenerator \
    PackageManager \
    DeployQML

OTHER_FILES += \
    ../README.md \
    ../LICENSE

win32{
    deploy.CONFIG = recursive
    QMAKE_EXTRA_TARGETS += deploy
}
