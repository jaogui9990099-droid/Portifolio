package dev.nextstars.meridian.domain.model;

import java.time.Duration;
import java.time.Instant;
import java.util.Optional;
import java.util.UUID;

public record ExecutionResult(
        UUID id,
        UUID jobId,
        int attempt,
        boolean success,
        String output,
        String errorMessage,
        Instant startedAt,
        Instant completedAt,
        Duration elapsed
) {

    public static ExecutionResult success(UUID jobId, int attempt, String output, Instant startedAt) {
        Instant now = Instant.now();
        return new ExecutionResult(UUID.randomUUID(), jobId, attempt, true, output, null,
                startedAt, now, Duration.between(startedAt, now));
    }

    public static ExecutionResult failure(UUID jobId, int attempt, String error, Instant startedAt) {
        Instant now = Instant.now();
        return new ExecutionResult(UUID.randomUUID(), jobId, attempt, false, null, error,
                startedAt, now, Duration.between(startedAt, now));
    }

    public Optional<String> errorMessageAsOptional() {
        return Optional.ofNullable(errorMessage);
    }
}
