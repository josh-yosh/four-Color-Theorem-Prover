#ifndef MAP_COLORING_H
#define MAP_COLORING_H

#include <set>
#include <vector>
#include <map>
#include "AtomicEnclosure.h"

using namespace std;

// Number of colors the four color theorem guarantees is always sufficient.
const int NUM_COLORS = 4;

// Builds an adjacency map between enclosures: two enclosures are adjacent
// (neighbors) when they share at least one edge.
map<AtomicEnclosure, set<AtomicEnclosure>> buildAdjacencyMap(const set<AtomicEnclosure>& enclosures);

// Returns true if it is safe to assign `color` to `region` given the current
// (partial) coloring -- i.e. no already-colored neighbor uses that color.
bool isSafeToColor(const AtomicEnclosure& region,
                   int color,
                   const map<AtomicEnclosure, set<AtomicEnclosure>>& adjacency,
                   const map<AtomicEnclosure, int>& coloring);

// Attempts to 4-color every enclosure via backtracking.
// On success returns true and fills `coloring` with a color (0..NUM_COLORS-1)
// for each enclosure. On failure returns false.
bool colorMap(const set<AtomicEnclosure>& enclosures,
              map<AtomicEnclosure, int>& coloring);

#endif // MAP_COLORING_H
