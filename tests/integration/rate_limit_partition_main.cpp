#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

#include <atomic>
#include <chrono>
#include <iostream>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

} // namespace

int main() {
    kurlyk::init(false);

    auto& manager = kurlyk::HttpRequestManager::get_instance();

    // --- Test 1: different keys are independent (count-based limit) ---
    {
        auto limit = manager.create_rate_limit(1, 60000);
        bool a = manager.allow_request(limit, limit, 1, "key_a", "key_a");
        require(a, "First request with key_a should be allowed");

        bool b = manager.allow_request(limit, limit, 2, "key_b", "key_b");
        require(b, "First request with key_b should be allowed (different key)");

        // Same key now blocked because count=1
        bool c = manager.allow_request(limit, limit, 3, "key_a", "key_a");
        require(!c, "Second request with key_a should be blocked (limit=1/60s)");

        manager.release_request(limit, limit, 1, "key_a", "key_a");
        manager.release_request(limit, limit, 2, "key_b", "key_b");
    }

    // --- Test 2: same key blocks (sequential mode) ---
    {
        auto limit = manager.create_rate_limit(10, 60000, true); // high count, sequential
        bool a = manager.allow_request(limit, limit, 4, "seq_same_key", "seq_same_key");
        require(a, "First sequential request with seq_same_key should be allowed");

        bool b = manager.allow_request(limit, limit, 5, "seq_same_key", "seq_same_key");
        require(!b, "Second sequential request with seq_same_key should be blocked");

        manager.release_request(limit, limit, 4, "seq_same_key", "seq_same_key");

        // After release, should be allowed again (count is far below 10)
        bool c = manager.allow_request(limit, limit, 6, "seq_same_key", "seq_same_key");
        require(c, "After release, sequential request should be allowed again");

        manager.release_request(limit, limit, 6, "seq_same_key", "seq_same_key");
    }

    // --- Test 3: empty key behaves as shared global state ---
    {
        auto limit = manager.create_rate_limit(1, 60000);
        bool a = manager.allow_request(limit, limit, 7, "", "");
        require(a, "First request with empty key should be allowed");

        bool b = manager.allow_request(limit, limit, 8, "", "");
        require(!b, "Second request with empty key should be blocked (shared state, limit=1/60s)");

        manager.release_request(limit, limit, 7, "", "");
    }

    // --- Test 4: sequential works inside a partition ---
    {
        auto limit = manager.create_rate_limit(10, 60000, true); // high count, sequential
        bool a = manager.allow_request(limit, limit, 9, "seq_partition", "seq_partition");
        require(a, "First sequential request with seq_partition should be allowed");

        bool b = manager.allow_request(limit, limit, 10, "seq_partition", "seq_partition");
        require(!b, "Second sequential request with seq_partition should be blocked");

        manager.release_request(limit, limit, 9, "seq_partition", "seq_partition");

        bool c = manager.allow_request(limit, limit, 11, "seq_partition", "seq_partition");
        require(c, "After release, sequential request with seq_partition should be allowed again");

        manager.release_request(limit, limit, 11, "seq_partition", "seq_partition");

        // Different key should not be blocked by seq_partition
        bool d = manager.allow_request(limit, limit, 12, "other_seq_partition", "other_seq_partition");
        require(d, "Sequential request with different key should be independent");

        manager.release_request(limit, limit, 12, "other_seq_partition", "other_seq_partition");
    }

    // --- Test 5: release_request only frees the correct partition ---
    {
        auto limit = manager.create_rate_limit(10, 60000, true); // sequential, high count
        bool a = manager.allow_request(limit, limit, 13, "rel_key", "rel_key");
        require(a, "First request with rel_key should be allowed");

        bool b = manager.allow_request(limit, limit, 14, "rel_key", "rel_key");
        require(!b, "Second request with rel_key should be blocked");

        // Release wrong key — should not unblock
        manager.release_request(limit, limit, 13, "wrong_key", "wrong_key");
        bool c = manager.allow_request(limit, limit, 15, "rel_key", "rel_key");
        require(!c, "Release of wrong key should not unblock rel_key");

        // Release correct key — should unblock
        manager.release_request(limit, limit, 13, "rel_key", "rel_key");
        bool d = manager.allow_request(limit, limit, 16, "rel_key", "rel_key");
        require(d, "Release of correct key should unblock rel_key");

        manager.release_request(limit, limit, 16, "rel_key", "rel_key");
    }

    // --- Test 6: time_until_next_allowed respects partitions ---
    {
        auto limit = manager.create_rate_limit(1, 60000);
        bool a = manager.allow_request(limit, limit, 17, "time_key_a", "time_key_a");
        require(a, "First request with time_key_a should be allowed");

        auto delay_a = manager.time_until_next_allowed(limit, limit, "time_key_a", "time_key_a");
        require(delay_a.count() > 0, "time_until_next_allowed for used key should be positive");

        auto delay_b = manager.time_until_next_allowed(limit, limit, "time_key_b", "time_key_b");
        require(delay_b.count() == 0, "time_until_next_allowed for unused key should be zero");

        manager.release_request(limit, limit, 17, "time_key_a", "time_key_a");
    }

    // --- Test 7: different general/specific handles with different keys ---
    {
        auto general = manager.create_rate_limit(1, 60000);
        auto specific = manager.create_rate_limit(10, 60000); // high count so specific never blocks

        bool a = manager.allow_request(general, specific, 18, "gen_k", "spec_k");
        require(a, "First request with different handles should be allowed");

        // Same general key should block even with different specific key
        bool b = manager.allow_request(general, specific, 19, "gen_k", "spec_k2");
        require(!b, "Same general key should block despite different specific key");

        // Different general key should pass (specific has high count)
        bool c = manager.allow_request(general, specific, 20, "gen_k2", "spec_k");
        require(c, "Different general key should pass when specific limit allows");

        manager.release_request(general, specific, 18, "gen_k", "spec_k");
        manager.release_request(general, specific, 20, "gen_k2", "spec_k");
    }

    // --- Test 8: double-release of same limit/key is idempotent ---
    {
        auto limit = manager.create_rate_limit(10, 60000, true);
        bool a = manager.allow_request(limit, limit, 21, "double_rel", "double_rel");
        require(a, "First request for double-release test should be allowed");

        manager.release_request(limit, limit, 21, "double_rel", "double_rel");
        manager.release_request(limit, limit, 21, "double_rel", "double_rel");

        bool b = manager.allow_request(limit, limit, 22, "double_rel", "double_rel");
        require(b, "After double-release, new request should be allowed");

        manager.release_request(limit, limit, 22, "double_rel", "double_rel");
    }

    kurlyk::deinit();

    std::cout << "Partitioned rate limit integration test passed" << std::endl;
    return 0;
}
