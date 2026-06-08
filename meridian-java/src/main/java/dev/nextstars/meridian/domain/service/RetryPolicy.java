package dev.nextstars.meridian.domain.service;

import java.time.Duration;
import java.util.concurrent.ThreadLocalRandom;

public final class RetryPolicy {

    private final double multiplier;
    private final Duration maxBackoff;

    public RetryPolicy(double multiplier, Duration maxBackoff) {
        if (multiplier < 1.0) throw new IllegalArgumentException("multiplier must be >= 1.0");
        this.multiplier = multiplier;
        this.maxBackoff = maxBackoff;
    }

    public Duration backoffFor(int attempt) {
        if (attempt <= 0) return Duration.ZERO;

        double baseSeconds = Math.pow(multiplier, attempt - 1);
        long cappedMillis  = Math.min((long)(baseSeconds * 1000), maxBackoff.toMillis());

        long jitterRange = (long)(cappedMillis * 0.2);
        long jitter = jitterRange > 0
                ? ThreadLocalRandom.current().nextLong(-jitterRange, jitterRange)
                : 0;

        long delayedMillis = Math.max(0, cappedMillis + jitter);
        return Duration.ofMillis(Math.min(delayedMillis, maxBackoff.toMillis()));
    }
}
