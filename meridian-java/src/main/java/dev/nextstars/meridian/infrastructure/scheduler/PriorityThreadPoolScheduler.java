package dev.nextstars.meridian.infrastructure.scheduler;

import dev.nextstars.meridian.domain.model.ExecutionResult;
import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.port.JobRepository;
import dev.nextstars.meridian.domain.port.JobScheduler;
import dev.nextstars.meridian.domain.service.RetryPolicy;
import dev.nextstars.meridian.infrastructure.config.SchedulerProperties;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.time.Instant;
import java.util.Comparator;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicLong;

@Component
public class PriorityThreadPoolScheduler implements JobScheduler {

    private static final Logger log = LoggerFactory.getLogger(PriorityThreadPoolScheduler.class);

    private final ThreadPoolExecutor executor;
    private final JobRepository repository;
    private final RetryPolicy retryPolicy;

    private final AtomicLong totalSubmitted = new AtomicLong();
    private final AtomicLong totalCompleted = new AtomicLong();
    private final AtomicLong totalFailed    = new AtomicLong();

    public PriorityThreadPoolScheduler(JobRepository repository,
                                       RetryPolicy retryPolicy,
                                       SchedulerProperties props) {
        this.repository  = repository;
        this.retryPolicy = retryPolicy;

        PriorityBlockingQueue<Runnable> queue = new PriorityBlockingQueue<>(
                props.maxQueueSize(),
                Comparator.comparingInt(r -> r instanceof PrioritizedTask t ? t.order() : Integer.MAX_VALUE)
        );

        this.executor = new ThreadPoolExecutor(
                props.poolSize(), props.poolSize(),
                0L, TimeUnit.MILLISECONDS,
                queue,
                new WorkerThreadFactory(),
                new ThreadPoolExecutor.AbortPolicy()
        );

        log.info("scheduler started -- pool={} maxQueue={}", props.poolSize(), props.maxQueueSize());
    }

    @Override
    public CompletableFuture<Void> enqueue(Job job) {
        totalSubmitted.incrementAndGet();
        CompletableFuture<Void> future = new CompletableFuture<>();
        executor.execute(new PrioritizedTask(job.priority().order(), () -> {
            executeJob(job);
            future.complete(null);
        }));
        return future;
    }

    private void executeJob(Job job) {
        Instant startedAt = Instant.now();
        job.markRunning();
        repository.save(job);

        try {
            String output = simulateExecution(job);
            ExecutionResult result = ExecutionResult.success(job.id(), job.attemptCount(), output, startedAt);
            job.recordExecution(result);
            repository.save(job);
            totalCompleted.incrementAndGet();
            log.info("job completed id={} attempt={} elapsed={}ms",
                    job.id(), job.attemptCount(), result.elapsed().toMillis());
        } catch (Exception ex) {
            ExecutionResult result = ExecutionResult.failure(job.id(), job.attemptCount(), ex.getMessage(), startedAt);
            job.recordExecution(result);
            repository.save(job);
            totalFailed.incrementAndGet();
            log.warn("job failed id={} attempt={}/{} error={}",
                    job.id(), job.attemptCount(), job.maxRetries(), ex.getMessage());
            if (job.isRetryable()) scheduleRetry(job);
        }
    }

    private void scheduleRetry(Job job) {
        long delayMs = retryPolicy.backoffFor(job.attemptCount()).toMillis();
        log.info("retry scheduled id={} delay={}ms", job.id(), delayMs);
        ScheduledExecutorHolder.scheduler.schedule(() -> enqueue(job), delayMs, TimeUnit.MILLISECONDS);
    }

    private String simulateExecution(Job job) throws Exception {
        long durationMs = switch (job.priority()) {
            case CRITICAL -> 50;
            case HIGH     -> 100;
            case NORMAL   -> 200;
            case LOW      -> 400;
        };
        Thread.sleep(durationMs);
        return "executed payload: " + job.payload();
    }

    @Override
    @PreDestroy
    public void shutdown() {
        log.info("shutting down scheduler...");
        executor.shutdown();
        ScheduledExecutorHolder.scheduler.shutdown();
        try {
            if (!executor.awaitTermination(30, TimeUnit.SECONDS))
                executor.shutdownNow();
        } catch (InterruptedException e) {
            executor.shutdownNow();
            Thread.currentThread().interrupt();
        }
        log.info("scheduler stopped -- submitted={} completed={} failed={}",
                totalSubmitted.get(), totalCompleted.get(), totalFailed.get());
    }

    @Override
    public SchedulerStats stats() {
        return new SchedulerStats(
                executor.getActiveCount(), executor.getCorePoolSize(),
                executor.getQueue().size(),
                totalSubmitted.get(), totalCompleted.get(), totalFailed.get()
        );
    }

    private static final class ScheduledExecutorHolder {
        static final ScheduledExecutorService scheduler =
                Executors.newSingleThreadScheduledExecutor(r -> {
                    Thread t = new Thread(r, "meridian-retry");
                    t.setDaemon(true);
                    return t;
                });
    }

    private record PrioritizedTask(int order, Runnable delegate) implements Runnable {
        @Override public void run() { delegate.run(); }
    }

    private static final class WorkerThreadFactory implements ThreadFactory {
        private int counter = 0;

        @Override
        public Thread newThread(Runnable r) {
            Thread t = new Thread(r, "meridian-worker-" + counter++);
            t.setDaemon(true);
            return t;
        }
    }
}
