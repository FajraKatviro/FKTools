Toolset for application development

  1. PackageManager - manage imageset settings for existed image art;
  2. PackageGenerator - build imagesets with settings provided by PackageManager;
  3. ImageManager - GUI application for visual control over PackageManager. Also allow create sets of iOS icons and splash screens from given images;
  4. DeployQML - copy QML dependencies on linux platforms.

Installation:
  - clone repository with --recursive option
  - configure project (src/FKTools.pro) with QtCreator
  - add 'make install' step after 'make' step
  - (windows only) add 'make deploy' step after 'make install' step
  - build
  - now your fresh binaries in user applications folder ($HOME/Applications/FKTools for mac, $HOME/FKTools for other unix, %HOMEPATH%\AppData\Roaming\FKTools for windows)

If you have any questions, write to FajraKatviro@gmail.com and I will be glad to answer
