#include "s1.hpp"
#include "nearest_search.hpp"
#ifdef PMEDIAN_PROFILE_SEQ
#include "phase_profile.hpp"
#endif
#include <utility>

namespace pmedian {

S1Result solve_s1(const Instance& instance, StationSet stations) {
#ifdef PMEDIAN_PROFILE_SEQ
    PhaseProfile nearest_profile("refresh_nearest");
    PhaseProfile distance_profile("candidate_distances");
    PhaseProfile swap_profile("trial_cost");
#endif
    NearestSearch search(instance, std::move(stations));
    std::vector<double> distances(instance.demand.size());
    {
#ifdef PMEDIAN_PROFILE_SEQ
        PhaseTimer timer(nearest_profile);
#endif
        search.initialize();
    }

    while (search.station_count() < instance.candidates.size()) {
        SwapMove best;
        for (std::size_t in = 0; in < instance.candidates.size(); ++in) {
            if (search.is_selected(in)) continue;
            // Odległości do in przydadzą się przy każdym możliwym out.
            {
#ifdef PMEDIAN_PROFILE_SEQ
                PhaseTimer timer(distance_profile);
#endif
                search.cache_candidate(in, distances);
            }
            for (std::size_t position = 0; position < search.station_count(); ++position) {
                SwapMove candidate;
                {
#ifdef PMEDIAN_PROFILE_SEQ
                    PhaseTimer timer(swap_profile);
#endif
                    candidate = search.evaluate(position, in, distances);
                }
                keep_best(best, candidate);
            }
        }
        if (!search.apply(best)) break;
#ifdef PMEDIAN_PROFILE_SEQ
        PhaseTimer timer(nearest_profile);
#endif
        search.refresh_nearest();
    }
    return search.finish();
}

} // namespace pmedian
