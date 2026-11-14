import QtQuick

QtObject {
    property var manager
    property color carColor: "#C8C9CA"
    property string carFinish: "lucida"
    property bool _ready: false

    function load() {
        _ready = false
        const meta = manager.currentMeta()
        carColor = meta.color || "#C8C9CA"
        carFinish = meta.finish || "lucida"
        _ready = true
        save()
    }
    function save() {
        if (_ready && manager) manager.saveCurrentMeta({ color: carColor.toString(), finish: carFinish })
    }
    onCarColorChanged: save()
    onCarFinishChanged: save()
}
