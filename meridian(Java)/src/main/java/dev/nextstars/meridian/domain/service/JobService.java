package dev.nextstars.meridian.domain.service;

import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;
import dev.nextstars.meridian.domain.port.JobRepository;
import dev.nextstars.meridian.domain.port.JobScheduler;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

@Service
public class JobService {

    private static final Logger log = LoggerFactory.getLogger(JobService.class);

    private final JobRepository repository;
    private final JobScheduler scheduler;

    public JobService(JobRepository repository, JobScheduler scheduler) {
        this.repository = repository;
        this.scheduler = scheduler;
    }

    public Job submit(Job job) {
        Job saved = repository.save(job);
        scheduler.enqueue(saved)
                .exceptionally(ex -> {
                    log.error("Failed to enqueue job {}: {}", saved.id(), ex.getMessage());
                    return null;
                });
        log.info("Submitted job id={} name={} priority={}", saved.id(), saved.name(), saved.priority());
        return saved;
    }

    public Optional<Job> findById(UUID id) {
        return repository.findById(id);
    }

    public List<Job> list(JobStatus status, JobPriority priority, int page, int size) {
        return repository.findAll(status, priority, page, size);
    }

    public boolean cancel(UUID id) {
        return repository.findById(id).map(job -> {
            try {
                job.cancel();
                repository.save(job);
                log.info("Cancelled job id={}", id);
                return true;
            } catch (IllegalStateException e) {
                log.warn("Cannot cancel job id={}: {}", id, e.getMessage());
                return false;
            }
        }).orElse(false);
    }

    public JobScheduler.SchedulerStats schedulerStats() {
        return scheduler.stats();
    }
}
