package dev.nextstars.meridian.domain.port;

import dev.nextstars.meridian.domain.model.Job;

import java.util.concurrent.CompletableFuture;

public interface JobScheduler {
    CompletableFuture<Void> enqueue(Job job);
    void shutdown();
    SchedulerStats stats();

    record SchedulerStats(
            int activeWorkers,
            int poolSize,
            long pendingQueueDepth,
            long totalSubmitted,
            long totalCompleted,
            long totalFailed
    ) {}
}
