package dev.nextstars.meridian.adapter.rest.dto;

import dev.nextstars.meridian.domain.model.ExecutionResult;
import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

public record JobResponse(
        UUID id,
        String name,
        String payload,
        JobStatus status,
        JobPriority priority,
        int maxRetries,
        int attemptCount,
        long timeoutSeconds,
        Instant submittedAt,
        Instant lastUpdatedAt,
        List<ExecutionSummary> executions
) {

    public record ExecutionSummary(
            UUID id,
            int attempt,
            boolean success,
            String output,
            String errorMessage,
            Instant startedAt,
            Instant completedAt,
            long elapsedMs
    ) {
        public static ExecutionSummary from(ExecutionResult r) {
            return new ExecutionSummary(
                    r.id(), r.attempt(), r.success(),
                    r.output(), r.errorMessageAsOptional().orElse(null),
                    r.startedAt(), r.completedAt(), r.elapsed().toMillis()
            );
        }
    }

    public static JobResponse from(Job job) {
        return new JobResponse(
                job.id(),
                job.name(),
                job.payload(),
                job.status(),
                job.priority(),
                job.maxRetries(),
                job.attemptCount(),
                job.timeoutSeconds(),
                job.submittedAt(),
                job.lastUpdatedAt(),
                job.executions().stream().map(ExecutionSummary::from).toList()
        );
    }
}
