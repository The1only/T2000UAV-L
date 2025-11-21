// GPX Used for simulation of GPS data, for testing.

#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QList>

struct TrackPoint {
    double latitude;
    double longitude;
    double elevation;
    double dt;
    QDateTime time;
    double speed = 0.0; // meters per second
};

class GpxParser {
public:
    bool parseFile(const QString &filePath);
    QList<TrackPoint> getTrackPoints() const;

private:
    QList<TrackPoint> trackPoints;
};

#endif // GPXPARSER_H
