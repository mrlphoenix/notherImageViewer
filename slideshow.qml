import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.2

Item {
    id: root

    signal nextItem()
    signal prevItem()
    signal nextRItem()
    signal prevRItem()
    signal randomItem()
    signal setDir(string dir)
    signal toggleFullScreen()
    signal copy()

    property bool randomMode: false
    function showItem(url)
    {
        img.source = url
        fileInfo.text = url
    }

    function next()
    {
        if (randomMode)
            nextRItem()
        else
            nextItem()
    }
    function prev()
    {
        if (randomMode)
            prevRItem()
        else
            prevItem()
    }

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        folder: shortcuts.pictures
        selectFolder: true
        onAccepted: {
            img.source = fileDialog.fileUrl
            if (selectFolder)
            {
                setDir(fileDialog.fileUrl.toString() + "/file.bin")
                next()
                prev()
            }
            else
                setDir(fileDialog.fileUrl.toString())
            root.focus = true
        }
        onRejected: {
            toggleFullScreen()
            toggleFullScreen()
        }
        nameFilters: [ "Image files (*.jpg *.png)" ]
        Component.onCompleted: {visible = true;
            fileDialog.close()}
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: "#000000"
    }
    MouseArea {
        id: globalClick
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: {
            console.log("pressed!")
            if (globalClick.pressedButtons & Qt.LeftButton)
                next()
            else if (globalClick.pressedButtons & Qt.RightButton)
                prev()
        }

        onWheel: {
            if (wheel.angleDelta.y < 0)
                next()
            else
                prev()
        }
    }

    AnimatedImage {
        id: img
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        onStatusChanged: playing = (status == AnimatedImage.Ready)
    }
    /*
    AnimatedImage {
        id: aimg
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }*/

    Text {
        id: randomIcon
        font.pointSize: 36
        text: "R "
        x: parent.width - randomIcon.width
        visible: randomMode
        color: "#FFFFFF"
        opacity: 0.5
    }
    Text {
        id: slideshowIcon
        font.pointSize: 16
        text:  "S (" + slideshowTimer.interval.toString() + ")"
        x: parent.width - slideshowIcon.width
        y: randomIcon.height + 4
        visible: slideShowActive
        color: "#FFFFFF"
        opacity: 0.5
    }


    Rectangle {
        id: fileInfoBG
        x: fileInfo.x - 10
        y: fileInfo.y - 2
        width: fileInfo.width + 20
        height: fileInfo.height + 2
        color: "#FFFFFF"
        opacity: 0.5
    }
    Text {
        id: fileInfo
        font.pointSize: 8
        x: parent.width/2 - fileInfo.width/2
        y: parent.height - fileInfo.height
    }

    Keys.onSpacePressed: {
        if (randomMode)
            randomItem()
        else
            nextItem()
    }
    Keys.onRightPressed: {
        next()
    }
    Keys.onLeftPressed:{
        prev()
    }
    Keys.onReturnPressed: {
        fileDialog.open()
    }
    Keys.onTabPressed:{
        randomMode = !randomMode
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_C)
            copy();

        else if (event.key === Qt.Key_S || event.key === Qt.Key_Shift)
        {
            if (slideShowActive)
            {
                slideshowTimer.stop()
                slideShowActive = false
            }
            else
            {
                slideshowTimer.restart()
                slideShowActive = true
            }
        }
        else if (event.key === Qt.Key_Plus)
        {
            slideshowTimer.interval = slideshowTimer.interval * 1.1 + 1
            if (slideShowActive)
                slideshowTimer.restart()
        }
        else if (event.key === Qt.Key_Minus)
        {
            slideshowTimer.interval = slideshowTimer.interval * 0.9
            if (slideShowActive)
                slideshowTimer.restart()
        }
        else if (event.key === Qt.Key_Control)
        {
            randomMode = !randomMode
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Alt || event.key === Qt.Key_AltGr)
        {
            toggleFullScreen()
        }
    }

    property bool slideShowActive: false
    Timer {
        id: slideshowTimer
        repeat: true
        interval: 5000
        onTriggered: next()
    }
}
