import QtQuick 2.5
import QtQuick.Window 2.2
import QtQml.Models 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import imageManager 1.0

Window {
    visible: true
    width:1200
    height:800

    property real headerRowHeight:100
    property real headerColumnWidth:200

    property real rowHeight:200
    property real columnWidth:200

    property int commonBorderWidth:1
    property int commonRadius:3

    ImageChecker{
        id:imageChecker
        onPackageManagerOutput: logOutput.append(output)
        //Component.onCompleted: refreshPackage()
    }

    ListView{
        id:mainView
        anchors.fill: parent
        header: headerDelegate
        delegate: rowDelegate
        model:imageChecker.model
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
                        text:imageChecker.packageFolder == "" ? "Select package folder" : imageChecker.packageFolder
                    }
                    MouseArea{
                        anchors.fill: parent
                        onClicked: pathSelector.open()
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
                        font.pointSize: 14
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
                            text: rowItem.text
                            font.pointSize: 14
                        }
                        CheckBox{
                            id:cropCheckbox
                            text:"Crop image"
                            checked: imageCrop
                            onCheckedChanged: imageCrop=checked
                            style: CheckBoxStyle{
                                label:Text{
                                    font.pointSize: 12
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
                        Text{
                            anchors.centerIn: parent
                            text:display
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
}

