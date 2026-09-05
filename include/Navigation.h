#pragma once

// Planned responsibility: Odometry, IMU integration and pose/movement estimation; IMU and data interfaces are TBD.
// Placeholder only: these methods currently perform no hardware operations.
class Navigation {
public:
    void begin();
    void update();
};
