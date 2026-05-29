package dev.nextstars.meridian.domain.service;

import org.junit.jupiter.api.Test;

import java.time.Duration;

import static org.assertj.core.api.Assertions.*;

class RetryPolicyTest {

    private final RetryPolicy policy = new RetryPolicy(2.0, Duration.ofSeconds(60));

    @Test
    void attempt0_returnsZeroDelay() {
        assertThat(policy.backoffFor(0)).isEqualTo(Duration.ZERO);
    }

    @Test
    void backoffGrowsExponentially() {
        Duration d1 = policy.backoffFor(1);
        Duration d2 = policy.backoffFor(2);
        Duration d3 = policy.backoffFor(3);

        // Allow for jitter — each subsequent delay should be at least 1.5x the previous.
        assertThat(d2.toMillis()).isGreaterThan((long)(d1.toMillis() * 1.5));
        assertThat(d3.toMillis()).isGreaterThan((long)(d2.toMillis() * 1.5));
    }

    @Test
    void backoffIsCappedAtMax() {
        // Attempt 100 — without a cap, backoff would be astronomical.
        Duration large = policy.backoffFor(100);
        assertThat(large.toSeconds()).isLessThanOrEqualTo(60);
    }

    @Test
    void invalidMultiplierThrows() {
        assertThatThrownBy(() -> new RetryPolicy(0.5, Duration.ofSeconds(60)))
                .isInstanceOf(IllegalArgumentException.class);
    }
}
