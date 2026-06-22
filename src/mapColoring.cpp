#include <set>
#include <vector>
#include <map>
#include "AtomicEnclosure.h"
#include "mapColoring.h"

using namespace std;

map<AtomicEnclosure, set<AtomicEnclosure>> buildAdjacencyMap(const set<AtomicEnclosure>& enclosures) {
    map<AtomicEnclosure, set<AtomicEnclosure>> adjacency;

    for (const AtomicEnclosure& a : enclosures) {
        for (const AtomicEnclosure& b : enclosures) {
            if (a == b) continue;
            if (a.sharesEdgeWith(b)) {
                adjacency[a].insert(b);
            }
        }
    }

    return adjacency;
}

bool isSafeToColor(const AtomicEnclosure& region,
                   int color,
                   const map<AtomicEnclosure, set<AtomicEnclosure>>& adjacency,
                   const map<AtomicEnclosure, int>& coloring) {
    auto it = adjacency.find(region);
    if (it == adjacency.end()) {
        return true; // no neighbors, anything is safe
    }

    for (const AtomicEnclosure& neighbor : it->second) {
        auto colored = coloring.find(neighbor);
        if (colored != coloring.end() && colored->second == color) {
            return false;
        }
    }

    return true;
}

// Recursive helper that colors enclosures one at a time.
static bool colorMapHelper(const vector<AtomicEnclosure>& regions,
                           size_t index,
                           const map<AtomicEnclosure, set<AtomicEnclosure>>& adjacency,
                           map<AtomicEnclosure, int>& coloring) {
    if (index == regions.size()) {
        return true; // every region has been colored
    }

    const AtomicEnclosure& region = regions[index];

    for (int color = 0; color < NUM_COLORS; ++color) {
        if (isSafeToColor(region, color, adjacency, coloring)) {
            coloring[region] = color;

            if (colorMapHelper(regions, index + 1, adjacency, coloring)) {
                return true;
            }

            coloring.erase(region); // backtrack
        }
    }

    return false;
}

bool colorMap(const set<AtomicEnclosure>& enclosures,
              map<AtomicEnclosure, int>& coloring) {
    coloring.clear();

    map<AtomicEnclosure, set<AtomicEnclosure>> adjacency = buildAdjacencyMap(enclosures);

    vector<AtomicEnclosure> regions(enclosures.begin(), enclosures.end());

    return colorMapHelper(regions, 0, adjacency, coloring);
}
