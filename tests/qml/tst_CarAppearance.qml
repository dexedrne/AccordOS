import QtQuick
import QtTest
import "../../qml" as UI

TestCase {
    name: "CarAppearance"
    QtObject {
        id: store
        property string current: "accord"
        property var metadata: ({})
        function currentMeta() { return metadata[current] || ({}) }
        function saveCurrentMeta(patch) { metadata[current] = Object.assign({}, currentMeta(), patch) }
    }
    UI.CarAppearance { id: appearance; manager: store }
    function init() {
        store.metadata = { accord: { color: "#973b35", finish: "opaca" }, civic: {} }
        store.current = "accord"
        appearance.load()
    }
    function test_editsAndSwitchesPreserveAppearance() {
        compare(appearance.carColor.toString(), "#973b35")
        appearance.carColor = "#123456"
        appearance.carFinish = "pastello"
        compare(store.currentMeta().color, "#123456")
        compare(store.currentMeta().finish, "pastello")
        store.current = "civic"
        appearance.load()
        compare(appearance.carColor.toString(), "#c8c9ca")
        compare(appearance.carFinish, "lucida")
        store.current = "accord"
        appearance.load()
        compare(appearance.carColor.toString(), "#123456")
        compare(appearance.carFinish, "pastello")
    }
}
