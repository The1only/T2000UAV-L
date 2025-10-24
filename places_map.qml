import QtQuick
import QtLocation
import QtPositioning

Rectangle {
    anchors.fill: parent

    property double zoomLevel: 15
    property double lat: 59.91
    property double lon: 10.75
    property double las_lat_1:lat
    property double las_lat_2:lat
    property double las_lat_3:lat
    property double las_lat_4:lat
    property double las_lat_5:lat
    property double las_lat_6:lat
    property double las_lat_7:lat
    property double las_lat_8:lat
    property double las_lat_9:lat
    property double las_lat_10:lat
    property double las_lon_1:lon
    property double las_lon_2:lon
    property double las_lon_3:lon
    property double las_lon_4:lon
    property double las_lon_5:lon
    property double las_lon_6:lon
    property double las_lon_7:lon
    property double las_lon_8:lon
    property double las_lon_9:lon
    property double las_lon_10:lon


    Plugin {
        id: mapPlugin
        name: "osm"
    }

    function debug(a) {
        console.log("Zoom Level: ", a);
    }

    function addMarker(latitude, longitude) {
        var Component = Qt.createComponent("qrc:///marker.qml")
        var item = Component.createObject(parent, {
                                              coordinate: QtPositioning.coordinate(latitude, longitude)
                                          })
        map.addMapItem(item)
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 15
        property geoCoordinate startCentroid

        center: QtPositioning.coordinate(lat,lon) // Oslo
        Component.onCompleted: {
            addMarker(lat,lon)
        }

        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                                 map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
                             }
            onScaleChanged: (delta) => {
                                map.zoomLevel += Math.log2(delta)
                                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                            }
            onRotationChanged: (delta) => {
                                   map.bearing -= delta
                                   map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                               }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        Timer {
            interval: 1000; running: true; repeat: true
            onTriggered: {
                // map.zoomLevel = zoomLevel
                map.zoomLevelChanged(true)
                //               debug(map.zoomLevel)

                map.center = QtPositioning.coordinate(lat,lon) // Oslo
                //                map.centerChanged(lat,lon)
                //    map.centerChanged(true);
                addMarker(lat,lon)
            }
        }

        WheelHandler {
            id: wheel
            // workaround for QTBUG-87646 / QTBUG-112394 / QTBUG-112432:
            // Magic Mouse pretends to be a trackpad but doesn't work with PinchHandler
            // and we don't yet distinguish mice and trackpads on Wayland either
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            rotationScale: 1/120
            property: "zoomLevel"

        }
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: {
                (delta) => map.pan(-delta.x, -delta.y)
            }
        }
        Shortcut {
            enabled: map.zoomLevel < map.maximumZoomLevel
            sequence: StandardKey.ZoomIn
            onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
        }
        Shortcut {
            enabled: map.zoomLevel > map.minimumZoomLevel
            sequence: StandardKey.ZoomOut
            onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
        }
    }
}


/*
import QtQuick
//import QtQuick 2.15
//import QtQuick 2.0
import QtPositioning
//import QtPositioning 5.5
import QtLocation
//import QtLocation 5.6
//import QtLocation 5.10

import QtQuick.Controls 2.5

Rectangle {
    anchors.fill: parent

    //! [Initialize Plugin]
    Plugin {
        id: myPlugin
        name: "osm"

        PluginParameter {
                    name: "osm.mapping.custom.host";
                    value: "https://tile.thunderforest.com/landscape/{z}/{x}/{y}.png?apikey=b1622199f9ab45889f7dfc9a3acf9f65"
//                    value: "https://tile.thunderforest.com/outdoors/{z}/{x}/{y}.png?apikey=b1622199f9ab45889f7dfc9a3acf9f65"
                }


        // PluginParameter {
        //         name: "osm.mapping.custom.host"

        //         // OSM plugin will auto-append if .png isn't suffix, and that screws up apikey which silently
        //         // fails authentication (only Wireshark revealed it)
        //         value: "http://tile.thunderforest.com/landscape/%z/%x/%y.png?apikey=b1622199f9ab45889f7dfc9a3acf9f65&fake=.png"
        //     }

    }
    //! [Initialize Plugin]

    //! [Current Location]
    PositionSource {
        id: positionSource
        property variant lastSearchPosition: locationOslo
        active: true
        updateInterval: 120000 // 2 mins
        onPositionChanged:  {
            var currentPosition = positionSource.position.coordinate
            map.center = currentPosition
            var distance = currentPosition.distanceTo(lastSearchPosition)
            if (distance > 500) {
                // 500m from last performed pizza search
                lastSearchPosition = currentPosition
                searchModel.searchArea = QtPositioning.circle(currentPosition)
                searchModel.update()
            }
        }
    }
    //! [Current Location]

    //! [PlaceSearchModel]
    property variant locationOslo: QtPositioning.coordinate( 59.93, 10.76)

    PlaceSearchModel {
        id: searchModel

        plugin: myPlugin

        searchTerm: "Pizza"
        searchArea: QtPositioning.circle(locationOslo)

        Component.onCompleted: update()
    }
    //! [PlaceSearchModel]

    //! [Places MapItemView]
    Map {
        id: map
        anchors.fill: parent
        plugin: myPlugin;
        center: locationOslo
        zoomLevel: 13

        MapItemView {
            model: searchModel
            delegate: MapQuickItem {
                coordinate: place.location.coordinate

                anchorPoint.x: image.width * 0.5
                anchorPoint.y: image.height

                sourceItem: Column {
                    Image { id: image; source: "marker.png" }
                    Text { text: title; font.bold: true }
                }
            }
        }
    }
    //! [Places MapItemView]

    Connections {
        target: searchModel
        onStatusChanged: {
            if (searchModel.status == PlaceSearchModel.Error)
                console.log(searchModel.errorString());
        }
    }
}
*/
