#include "../../src/core/CarModelManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QTest>
#include <QtEndian>

class CarModelTests : public QObject {
    Q_OBJECT
private:
    QTemporaryDir project;
    QString fixture(const QString &name, const QJsonArray &nodes = QJsonArray{QJsonObject{{"name", "body"}}}) {
        const QString path = project.filePath(name + ".glb");
        const QJsonObject scene{{"asset", QJsonObject{{"version", "2.0"}}},
            {"scenes", QJsonArray{QJsonObject{{"nodes", QJsonArray{0}}}}},
            {"nodes", nodes}};
        QByteArray json = QJsonDocument(scene).toJson(QJsonDocument::Compact);
        while (json.size() % 4) json.append(' ');
        QByteArray bytes("glTF");
        auto word = [&](quint32 value) { char data[4]; qToLittleEndian(value, data); bytes.append(data, 4); };
        word(2); word(20 + json.size()); word(json.size()); bytes.append("JSON"); bytes.append(json);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) return {};
        return path;
    }
private slots:
    void init() {
        const QString cars = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/car";
        QVERIFY(QDir(cars).removeRecursively());
    }
    void importsPreserveCarsAndParts() {
        CarModelManager manager(project.path());
        QCOMPARE(manager.installCar(fixture("Accord")), QString());
        const QString accord = manager.currentId();
        QVERIFY(manager.saveParts({{"hood", QStringList{"bonnet"}}}));
        manager.saveCurrentMeta({{"color", "#aa2020"}, {"layout", QVariantMap{{"carScale", 1.2}}}});
        QCOMPARE(manager.installCar(fixture("Civic")), QString());
        QCOMPARE(manager.cars().size(), 2);
        QVERIFY(QFile::exists(manager.cars().first().toMap()["file"].toString()));
        QVERIFY(manager.selectCar(accord));
        QCOMPARE(manager.currentMeta()["color"].toString(), QString("#aa2020"));
        QCOMPARE(manager.currentParts()["parts"].toMap()["hood"].toStringList(), QStringList{"bonnet"});
        CarModelManager reopened(project.path());
        QCOMPARE(reopened.currentId(), accord);
        QVERIFY(!manager.selectCar("../../outside"));
        QCOMPARE(manager.currentId(), accord);
        QVERIFY(!manager.installCar(project.filePath("missing.glb")).isEmpty());
        QCOMPARE(manager.cars().size(), 2);
        QCOMPARE(manager.currentId(), accord);
    }
    void seedsAccordOnce() {
        QTemporaryDir defaults;
        QVERIFY(QDir().mkpath(defaults.filePath("assets/garage")));
        QVERIFY(QFile::copy(fixture("defaultAccord"), defaults.filePath("assets/garage/accord.glb")));
        QVERIFY(QFile::copy(fixture("defaultCivic"), defaults.filePath("assets/garage/civic.glb")));
        const QJsonArray cars{
            QJsonObject{{"file", "accord.glb"}, {"name", "1993 Honda Accord"}, {"color", "#a52a2a"}},
            QJsonObject{{"file", "missing.glb"}, {"name", "Unavailable"}},
            QJsonObject{{"file", "civic.glb"}, {"name", "1993 Honda Civic"}}};
        QFile catalog(defaults.filePath("assets/garage/catalog.json"));
        QVERIFY(catalog.open(QIODevice::WriteOnly));
        catalog.write(QJsonDocument(cars).toJson()); catalog.close();
        CarModelManager seeded(defaults.path());
        QCOMPARE(seeded.cars().size(), 2);
        QCOMPARE(seeded.currentMeta()["name"].toString(), QString("1993 Honda Accord"));
        QCOMPARE(seeded.currentMeta()["color"].toString(), QString("#a52a2a"));
        QVERIFY(seeded.configured());
        const QString civic = seeded.cars().last().toMap()["id"].toString();
        QVERIFY(seeded.selectCar(civic));
        CarModelManager restarted(defaults.path());
        QCOMPARE(restarted.cars().size(), 2);
        QCOMPARE(restarted.currentId(), civic);
    }
    void detectsCb7PartNames() {
        CarModelManager manager(project.path());
        QJsonArray nodes;
        for (const QString &name : {"body", "bonnet", "bootlid", "door_FL", "door_FR", "hubcap_FL", "hubcap_FR", "hubcap_RL", "hubcap_RR"})
            nodes.append(QJsonObject{{"name", name}, {"translation", QJsonArray{0, 1, 1}}});
        QJsonArray children;
        for (int i = 1; i < nodes.size(); ++i) children.append(i);
        QJsonObject body = nodes[0].toObject(); body["children"] = children; nodes[0] = body;
        const QVariantMap parts = manager.inspect(fixture("CB7", nodes))["parts"].toMap();
        QCOMPARE(parts["hood"].toStringList(), QStringList{"bonnet"});
        QCOMPARE(parts["trunk"].toStringList(), QStringList{"bootlid"});
        QCOMPARE(parts["doorL"].toStringList(), QStringList{"door_FL"});
        QCOMPARE(parts["doorR"].toStringList(), QStringList{"door_FR"});
        QCOMPARE(parts["wheels"].toStringList().size(), 4);
    }
    void rejectsTruncatedModelWithoutChangingGarage() {
        CarModelManager manager(project.path());
        QCOMPARE(manager.installCar(fixture("Accord")), QString());
        const QString selected = manager.currentId();
        const QString damaged = fixture("truncated");
        QFile file(damaged);
        QVERIFY(file.open(QIODevice::ReadWrite));
        const quint32 declared = qToLittleEndian(quint32(file.size() + 100));
        QVERIFY(file.seek(8));
        QCOMPARE(file.write(reinterpret_cast<const char *>(&declared), 4), qint64(4));
        file.close();
        QVERIFY(!manager.installCar(damaged).isEmpty());
        QCOMPARE(manager.currentId(), selected);
        QCOMPARE(manager.cars().size(), 1);
    }
};

int main(int argc, char **argv) {
    QTemporaryDir config;
    if (!config.isValid()) return 1;
    qputenv("XDG_CONFIG_HOME", config.path().toUtf8());
    QCoreApplication app(argc, argv);
    app.setApplicationName("Accord OS tests");
    app.setOrganizationName("VoltaOS");
    CarModelTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "tst_CarModelManager.moc"
