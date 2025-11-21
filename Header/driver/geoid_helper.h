#ifndef GEOID_HELPER_H
#define GEOID_HELPER_H

// Same as abou, will fix later...
#pragma once
#include <optional>
#include <string>

struct GeoidResult {
    double N;             // geoid separation (meters)
    double h_compensated; // h_ellipsoid - alpha * N
};

class GeoidHelper {
public:
    // Initialize with path to geoid grid directory or "" to use default search
    GeoidHelper(const std::string& geoidDataPath = "");
    // alpha: 1.0 -> full AMSL (h - N), 0.0 -> no compensation
    std::optional<GeoidResult> compensatedHeight(double lat_deg, double lon_deg,
                                                 double h_ellipsoid_m,
                                                 double alpha = 1.0);
    bool isValid() const;
private:
    struct Impl;
    Impl* impl = nullptr;
};

#endif // GEOID_HELPER_H
