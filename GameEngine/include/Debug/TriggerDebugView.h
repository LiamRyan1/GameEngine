#pragma once
#include <vector>

class Trigger;

struct TriggerDebugView
{
    int totalTriggers = 0;
    int enabledTriggers = 0;
    int disabledTriggers = 0;

    int goalZoneCount = 0;
    int deathZoneCount = 0;
    int checkpointCount = 0;
    int teleportCount = 0;
    int speedZoneCount = 0;
    int customCount = 0;

    std::vector<Trigger*> allTriggers;

	// UI state for trigger list
    // Currently selected trigger for editing (nullptr = none selected)
    Trigger* selectedTrigger = nullptr;

    // UI state for creating new triggers
    int selectedTriggerTypeIndex = 0;  // Index into trigger type dropdown
    char newTriggerName[128] = "Trigger_1";
    float newTriggerPosition[3] = { 0.0f, 2.0f, 0.0f };
    float newTriggerSize[3] = { 2.0f, 2.0f, 2.0f };

    // Type-specific parameters
    // For TELEPORT triggers
    float teleportDestination[3] = { 0.0f, 0.0f, 0.0f };

    // For SPEED_ZONE triggers
    float forceDirection[3] = { 0.0f, 1.0f, 0.0f };
    float forceMagnitude = 10.0f;
};