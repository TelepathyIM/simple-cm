#include "JsonUtils.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QLoggingCategory>

namespace SimpleCM {

QByteArray JsonUtils::messageToJson(const Tp::MessagePartList &message)
{
    QJsonArray array;
    for (const Tp::MessagePart &part : message) {
        QVariantMap map;
        for (const QString &key : part.keys()) {
            map.insert(key, part.value(key).variant());
        }
        QJsonObject partObject = QJsonObject::fromVariantMap(map);
        array.append(partObject);
    }

    return QJsonDocument(array).toJson(QJsonDocument::Indented);
}

Tp::MessagePartList JsonUtils::messageFromJson(const QByteArray &json)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (!doc.isArray()) {
        qWarning() << "Invalid: not an array";
        return {};
    }

    Tp::MessagePartList message;
    for (const QJsonValue &v : doc.array()) {
        if (v.type() != QJsonValue::Object) {
            qWarning() << "Invalid: A part is not an object";
            return {};
        }
        const QJsonObject partObject = v.toObject();
        const QVariantMap partMap = partObject.toVariantMap();

        Tp::MessagePart part;
        for (const QString &key : partMap.keys()) {
            part[key] = QDBusVariant(partMap.value(key));
        }

        message << part;
    }

    return message;
}

} // SimpleCM
