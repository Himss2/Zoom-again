#include "Zoom/ZoomState.hpp"

namespace zoom {
namespace {

std::atomic<bool> g_active{false};
std::atomic<bool> g_releasing{false};
std::atomic<float> g_factor{kNeutralZoom};
std::atomic<int> g_ownerPointerId{-1};

} // namespace

bool IsActive() { return g_active.load(std::memory_order_relaxed); }
void SetActive(bool active) { g_active.store(active, std::memory_order_relaxed); }

bool IsReleasing() { return g_releasing.load(std::memory_order_relaxed); }
void SetReleasing(bool releasing) { g_releasing.store(releasing, std::memory_order_relaxed); }

float GetFactor() { return g_factor.load(std::memory_order_relaxed); }
void SetFactor(float factor) { g_factor.store(factor, std::memory_order_relaxed); }

void AdjustFactor(float delta) {
    float f = g_factor.load(std::memory_order_relaxed) + delta;
    if (f < kMinZoom) f = kMinZoom;
    if (f > kMaxZoom) f = kMaxZoom;
    g_factor.store(f, std::memory_order_relaxed);
}

int GetOwnerPointerId() { return g_ownerPointerId.load(std::memory_order_relaxed); }
void SetOwnerPointerId(int pointerId) { g_ownerPointerId.store(pointerId, std::memory_order_relaxed); }

} // namespace zoom
