import QtQuick 2.5
import QtQuick.Window 2.2
import QtQml.Models 2.2

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
    }

    ListView{
        id:mainView
        anchors.fill: parent
        header: headerDelegate
        delegate: rowDelegate
        model:imageChecker.model
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
                    color:"yellow"
                    border.width: commonBorderWidth
                    radius:commonRadius
                    Text{
                        anchors.centerIn: parent
                        text: rowItem.text
                        font.pointSize: 14
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
}

