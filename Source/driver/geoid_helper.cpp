#include "geoid_helper.h"
#include <iostream>

#ifdef USE_GEOGRAPHICLIB
// GeographicLib headers
//#include <Geoid.hpp>
#include <GeographicLib/Geoid.hpp>
using namespace GeographicLib;
#endif

struct GeoidHelper::Impl {
#ifdef USE_GEOGRAPHICLIB
    std::unique_ptr<Geoid> geoid;
#endif
    bool valid = false;
};

GeoidHelper::GeoidHelper(const std::string& geoidDataPath)
    : impl(new Impl())
{
#ifdef USE_GEOGRAPHICLIB
    try {
        // Try EGM96 first; you may use "egm2008-1" if you include that grid
        const std::string model = "egm96-5";
        if (geoidDataPath.empty()) {
            impl->geoid = std::make_unique<Geoid>(model);
        } else {
            impl->geoid = std::make_unique<Geoid>(model, geoidDataPath);
        }
        impl->valid = true;
    } catch (const std::exception& e) {
        std::cerr << "Geoid init failed: " << e.what() << "\n";
        impl->valid = false;
    }
#else
    // No GeographicLib available: impl stays invalid
    impl->valid = false;
#endif
}

std::optional<GeoidResult> GeoidHelper::compensatedHeight(double lat_deg, double lon_deg,
                                                          double h_ellipsoid_m,
                                                          double alpha)
{
    if (!impl->valid) return std::nullopt;
#ifdef USE_GEOGRAPHICLIB
    double N = impl->geoid->operator()(lat_deg, lon_deg);
    GeoidResult r;
    r.N = N;
    r.h_compensated = h_ellipsoid_m - alpha * N;
    return r;
#else
    (void)lat_deg; (void)lon_deg; (void)h_ellipsoid_m; (void)alpha;
    return std::nullopt;
#endif
}

bool GeoidHelper::isValid() const { return impl && impl->valid; }
