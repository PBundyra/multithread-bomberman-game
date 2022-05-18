#include "map.h"

void Map::apply_changes(const vector <shared_ptr<Event>> events) {
    for (auto &event: events) {
        map.apply_changes(event);
    }
}
