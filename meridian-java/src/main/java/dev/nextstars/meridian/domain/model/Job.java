package dev.nextstars.meridian.domain.model;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

public class Job {

    private final UUID id;
    private final String name;
    private final String payload;
    private final JobPriority priority;
    private final int maxRetries;
    private final long timeoutSeconds;
    private final Instant submittedAt;

    private volatile JobStatus status;
    private volatile int attemptCount;
    private volatile Instant lastUpdatedAt;
    private final List<ExecutionResult> executions;

    private Job(Builder builder) {
        this.id             = builder.id;
        this.name           = builder.name;
        this.payload        = builder.payload;
        this.priority       = builder.priority;
        this.maxRetries     = builder.maxRetries;
        this.timeoutSeconds = builder.timeoutSeconds;
        this.submittedAt    = builder.submittedAt;
        this.status         = JobStatus.PENDING;
        this.attemptCount   = 0;
        this.lastUpdatedAt  = this.submittedAt;
        this.executions     = new ArrayList<>();
    }

    public UUID id()              { return id; }
    public String name()          { return name; }
    public String payload()       { return payload; }
    public JobPriority priority() { return priority; }
    public int maxRetries()       { return maxRetries; }
    public long timeoutSeconds()  { return timeoutSeconds; }
    public Instant submittedAt()  { return submittedAt; }
    public JobStatus status()     { return status; }
    public int attemptCount()     { return attemptCount; }
    public Instant lastUpdatedAt(){ return lastUpdatedAt; }

    public List<ExecutionResult> executions() {
        return Collections.unmodifiableList(executions);
    }

    public synchronized void markRunning() {
        if (status != JobStatus.PENDING)
            throw new IllegalStateException("cannot start job in state: " + status);
        this.status = JobStatus.RUNNING;
        this.attemptCount++;
        this.lastUpdatedAt = Instant.now();
    }

    public synchronized void recordExecution(ExecutionResult result) {
        executions.add(result);
        this.lastUpdatedAt = Instant.now();

        if (result.success()) {
            this.status = JobStatus.COMPLETED;
        } else if (attemptCount > maxRetries) {
            this.status = JobStatus.DEAD_LETTERED;
        } else {
            this.status = JobStatus.PENDING;
        }
    }

    public synchronized void cancel() {
        if (status.isTerminal())
            throw new IllegalStateException("cannot cancel job in terminal state: " + status);
        this.status        = JobStatus.CANCELLED;
        this.lastUpdatedAt = Instant.now();
    }

    public boolean isRetryable() {
        return status == JobStatus.PENDING && attemptCount <= maxRetries;
    }

    public static Builder builder() {
        return new Builder();
    }

    public static final class Builder {
        private UUID id             = UUID.randomUUID();
        private String name;
        private String payload;
        private JobPriority priority = JobPriority.NORMAL;
        private int maxRetries       = 3;
        private long timeoutSeconds  = 300;
        private Instant submittedAt  = Instant.now();

        public Builder id(UUID id)            { this.id = id; return this; }
        public Builder name(String name)      { this.name = name; return this; }
        public Builder payload(String p)      { this.payload = p; return this; }
        public Builder priority(JobPriority p){ this.priority = p; return this; }
        public Builder maxRetries(int n)      { this.maxRetries = n; return this; }
        public Builder timeoutSeconds(long s) { this.timeoutSeconds = s; return this; }
        public Builder submittedAt(Instant t) { this.submittedAt = t; return this; }

        public Job build() {
            if (name == null || name.isBlank())
                throw new IllegalArgumentException("job name is required");
            return new Job(this);
        }
    }
}
