import QtQuick 2.5
import QtQuick.Window 2.2
import QtQml.Models 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import imageManager 1.0

ApplicationWindow {
    visible: true
    width:1200
    height:800

    property real headerRowHeight:100
    property real headerColumnWidth:200

    property real rowHeight:200
    property real columnWidth:200

    property int commonBorderWidth:1
    property int commonRadius:3

    property int spriteWidth:100
    property int spriteHeight:200

    ImageChecker{
        id:imageChecker
        onPackageManagerOutput: logOutput.append(output)
    }

    menuBar: MenuBar {
        Menu {
            title: "Package"
            MenuItem { text: "Load..."; shortcut: StandardKey.Open; onTriggered: pathSelector.open() }
            MenuItem { text: "Save"; shortcut: StandardKey.Save; onTriggered: imageChecker.refreshPackage() }
            MenuItem { text: "Spawn..."; shortcut: StandardKey.New; onTriggered: targetSelector.open() }
        }
        Menu {
            title: "Deploy images"
            MenuItem { text: "iOS icons"; onTriggered: deployImageCreationRequest.newRequest("ios_icons") }
            MenuItem { text: "iOS splash screens";  onTriggered: deployImageCreationRequest.newRequest("ios_splashscreens") }
            MenuItem { text: "android icons"; onTriggered: deployImageCreationRequest.newRequest("android_icons") }
            MenuItem { text: "android splash screens";  onTriggered: deployImageCreationRequest.newRequest("android_splashscreens") }
        }
        Menu {
            title: "Sprites"
            MenuItem { text: "Select sprite size"; onTriggered: spriteSizeSelector.open() }
            MenuItem { text: "Merge spritesheet"; onTriggered: spriteSourceSelector.open() }
        }
    }

    Rectangle{
        anchors.fill: parent
        ListView{
            id:mainView
            anchors.fill: parent
            header: headerDelegate
            delegate: rowDelegate
            model:imageChecker.model
        }
    }

    FileDialog{
        id:pathSelector
        title:"Choose package folder"
        folder:"."
        selectFolder: true
        selectExisting: true
        selectMultiple: false
        onAccepted: imageChecker.setPackageUrl(fileUrl)
    }

    FileDialog{
        id:targetSelector
        title:"Choose target folder"
        folder:".."
        selectFolder: true
        selectExisting: true
        selectMultiple: false
        onAccepted: imageChecker.spawnPackage(fileUrl)
    }

    FileDialog{
        id:spriteSourceSelector
        title:"Choose sprites folder"
        folder:"."
        selectFolder: true
        selectExisting: true
        selectMultiple: false
        property url lastSelected
        onAccepted: {
            lastSelected=fileUrl
            spriteTargetSelector.open()
        }
    }

    FileDialog{
        id:spriteTargetSelector
        title:"Choose target folder"
        folder:"."
        selectFolder: true
        selectExisting: true
        selectMultiple: false
        onAccepted: imageChecker.createSpriteSheet(spriteSourceSelector.lastSelected,fileUrl,spriteWidth,spriteHeight)
    }

    Dialog{
        id:spriteSizeSelector
        title:"Select size for sprites"
        Column{
            SpinBox{
                value:spriteWidth
                onValueChanged: spriteWidth=value
                minimumValue: 1
                maximumValue: 1024
            }
            SpinBox{
                value:spriteHeight
                onValueChanged: spriteHeight=value
                minimumValue: 1
                maximumValue: 1024
            }
        }
        standardButtons:  StandardButton.Ok
    }

    Dialog{
        id:sizesetSelector
        title:"Select size to add"
        TextField{
            id:inputField
            placeholderText: "800x600"
            validator: RegExpValidator{regExp:/\d+x\d+/}
        }
        standardButtons: inputField.acceptableInput ? StandardButton.Ok | StandardButton.Cancel : StandardButton.Cancel
        onAccepted: imageChecker.addSizeset(inputField.text)
    }

    Component{
        id:headerDelegate
        Item{
            width:mainView.width
            height: headerRowHeight
            ListView{
                anchors.fill: parent
                orientation: Qt.Horizontal
                model:imageChecker.sizes
                interactive: false
                header: Rectangle{
                    width:headerColumnWidth
                    height:headerRowHeight
                    color:"lightgrey"
                    border.width: commonBorderWidth
                    radius:commonRadius
                    Text{
                        anchors.centerIn: parent
                        text:imageChecker.packageFolder == "" ? "Load package to start" : imageChecker.packageFolder
                    }
                }
                delegate: Rectangle {
                    width:columnWidth
                    height: headerRowHeight
                    color:"lightblue"
                    border.width: commonBorderWidth
                    radius:commonRadius
                    Text{
                        anchors.centerIn: parent
                        text:modelData
                        font.pixelSize: 14
                    }
                    Button{
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 10
                        text:"Remove sizeset"
                        onClicked: imageChecker.removeSizeset(modelData)
                    }
                }
                footer:Rectangle{
                    width:headerColumnWidth
                    height:headerRowHeight
                    color:"lightblue"
                    border.width: commonBorderWidth
                    radius:commonRadius
                    Text{
                        anchors.centerIn: parent
                        text:"Add size set..."
                    }
                    MouseArea{
                        anchors.fill: parent
                        onClicked: sizesetSelector.open()
                    }
                }
            }
        }
    }

    Component{
        id:rowDelegate
        Item{
            id:rowItem
            height:rowHeight
            width:mainView.width
            property string text:display
            property var childIndex:selfIndex
            property var avaliableSizes:sourceSizes
            ListView{
                anchors.fill: parent
                orientation: Qt.Horizontal
                interactive: false
                header: Rectangle{
                    width:headerColumnWidth
                    height: rowHeight
                    //color:"yellow"
                    border.width: commonBorderWidth
                    radius:commonRadius
                    Column{
                        anchors.centerIn: parent
                        Text{
                            id:imageName
                            width:headerColumnWidth - 10
                            text: rowItem.text
                            font.pixelSize: 14
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }
                        CheckBox{
                            id:cropCheckbox
                            text:"Crop image"
                            checked: imageCrop
                            onCheckedChanged: imageCrop=checked
                            style: CheckBoxStyle{
                                label:Text{
                                    font.pixelSize: 12
                                    text:control.text
                                }
                            }
                        }
                    }
                }
                model:DelegateModel{
                    model:imageChecker.model
                    delegate: Item {
                        width:columnWidth
                        height: rowHeight
                        Image{
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            source: "file:///" + WorkingFolder + "/" + imageChecker.packageFolder + "/" + display + "/" + rowItem.text
                        }
                        ComboBox{
                            property var currentSize:display
                            anchors{
                                left:parent.left
                                right:parent.right
                                bottom: parent.bottom
                            }
                            model: ListModel{
                                Component.onCompleted:{
                                    append({"text":"Auto ..... (%1)".arg(autoSizeValue)})
                                    for(var i=0;i<rowItem.avaliableSizes.length;++i){
                                        append({"text":rowItem.avaliableSizes[i]})
                                    }
                                }
                            }
                            Component.onCompleted:{
                                selectIndex()
                                onCurrentSizeChanged.connect(function(){
                                    selectIndex()
                                })
                                onCurrentIndexChanged.connect(function(){
                                    if(currentIndex===0)
                                        autoSize=true
                                    else{
                                        customSize=currentText
                                        autoSize=false
                                    }
                                })
                            }
                            function selectIndex(){
                                if(autoSize)
                                    currentIndex=0
                                else
                                    currentIndex=find(customSize)
                            }
                        }
                    }
                    rootIndex: rowItem.childIndex
                }
            }
        }
    }
    TextArea{
        id:logOutput
        anchors.fill: parent
        visible: false
        readOnly: true
    }
    Button{
        anchors{
            right:parent.right
            bottom:parent.bottom
        }
        text:!logOutput.visible ? "Show log" : "Hide log"
        onClicked: logOutput.visible = !logOutput.visible
    }

    //deploy image creation
    QtObject{
        id: deployImageCreationRequest
        property string imageTemplate
        property url deployImageSource
        property url deployImageTarget
        function newRequest(templateName){
            imageTemplate = templateName
            iconSelector.open()
        }
        function createImages(){
            imageChecker.spawnImage(deployImageSource,deployImageTarget,imageTemplate)
        }
    }

    FileDialog{
        id:iconSelector
        title:"Choose source image"
        folder:".."
        selectExisting: true
        selectMultiple: false
        onAccepted: {
            deployImageCreationRequest.deployImageSource = fileUrl
            iconTargetFolderSelector.open()
        }
    }

    FileDialog{
        id:iconTargetFolderSelector
        title:"Choose target folder"
        folder:".."
        selectFolder: true
        selectExisting: true
        selectMultiple: false
        onAccepted: {
            deployImageCreationRequest.deployImageTarget = fileUrl
            deployImageCreationRequest.createImages()
        }
    }

}

