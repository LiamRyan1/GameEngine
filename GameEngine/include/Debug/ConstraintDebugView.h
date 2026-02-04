#pragma once

#include <vector>
#include <string>

class Constraint;

// Read-only view of constraint system state for debug UI
struct ConstraintDebugView
{
    int totalConstraints = 0;
    int activeConstraints = 0;
    int brokenConstraints = 0;

    // Constraint counts by type
    int fixedCount = 0;
    int hingeCount = 0;
    int sliderCount = 0;
    int springCount = 0;
    int dof6Count = 0;

    // All constraints for listing
    std::vector<Constraint*> allConstraints;
};