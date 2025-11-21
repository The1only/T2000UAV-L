#include "gpx_parse.h"
#include <QFile>
#include <QDomDocument>
#include <QtMath>
#include <QDebug>

static constexpr double EARTH_RADIUS = 6371000.0; // in meters

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLon = qDegreesToRadians(lon2 - lon1);
    lat1 = qDegreesToRadians(lat1);
    lat2 = qDegreesToRadians(lat2);

    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(lat1) * qCos(lat2) *
                   qSin(dLon / 2) * qSin(dLon / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    return EARTH_RADIUS * c;
}

bool GpxParser::parseFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Failed to parse XML content";
        file.close();
        return false;
    }
    file.close();

    QDomNodeList trkptNodes = doc.elementsByTagName("trkpt");
    TrackPoint lastPoint;

    for (int i = 0; i < trkptNodes.count(); ++i) {
        QDomElement el = trkptNodes.at(i).toElement();
        TrackPoint pt;
        pt.latitude = el.attribute("lat").toDouble();
        pt.longitude = el.attribute("lon").toDouble();

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();
            if (childEl.tagName() == "ele") {
                pt.elevation = childEl.text().toDouble();
            } else if (childEl.tagName() == "time") {
                pt.time = QDateTime::fromString(childEl.text(), Qt::ISODate);
                pt.time.setTimeSpec(Qt::UTC);
            } else if (childEl.tagName() == "speed") {
                pt.speed = childEl.text().toDouble();
            }
            child = child.nextSibling();
        }

        // dt calculation
        if (!trackPoints.isEmpty()) {
            double distance = haversineDistance(
                lastPoint.latitude, lastPoint.longitude,
                pt.latitude, pt.longitude
                );

            qint64 timeDiff = lastPoint.time.msecsTo(pt.time);
            if (timeDiff > 0) {
                pt.dt = timeDiff / 1000.0; // meters per second
            }
        }

        lastPoint = pt;
        trackPoints.append(pt);
    }

    return true;
}

QList<TrackPoint> GpxParser::getTrackPoints() const {
    return trackPoints;
}

