package dev.nextstars.meridian.domain.service;

import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;
import dev.nextstars.meridian.domain.port.JobRepository;
import dev.nextstars.meridian.domain.port.JobScheduler;
import dev.nextstars.meridian.infrastructure.persistence.InMemoryJobRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;

import static org.assertj.core.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

class JobServiceTest {

    private JobRepository repository;
    private JobScheduler scheduler;
    private JobService service;

    @BeforeEach
    void setUp() {
        repository = new InMemoryJobRepository();
        scheduler = mock(JobScheduler.class);
        when(scheduler.enqueue(any())).thenReturn(CompletableFuture.completedFuture(null));
        service = new JobService(repository, scheduler);
    }

    @Test
    void submit_persistsJobAndEnqueues() {
        Job job = Job.builder().name("test-job").payload("{}").build();

        Job result = service.submit(job);

        assertThat(result.id()).isNotNull();
        assertThat(result.status()).isEqualTo(JobStatus.PENDING);
        assertThat(repository.findById(result.id())).isPresent();
        verify(scheduler).enqueue(any());
    }

    @Test
    void submit_defaultsToNormalPriority() {
        Job job = Job.builder().name("default-priority-job").build();

        Job result = service.submit(job);

        assertThat(result.priority()).isEqualTo(JobPriority.NORMAL);
    }

    @Test
    void findById_returnsEmptyForUnknownId() {
        assertThat(service.findById(java.util.UUID.randomUUID())).isEmpty();
    }

    @Test
    void cancel_succeedsForPendingJob() {
        Job job = service.submit(Job.builder().name("cancel-me").build());

        boolean cancelled = service.cancel(job.id());

        assertThat(cancelled).isTrue();
        assertThat(repository.findById(job.id()).map(Job::status))
                .contains(JobStatus.CANCELLED);
    }

    @Test
    void cancel_returnsFalseForNonExistentJob() {
        assertThat(service.cancel(java.util.UUID.randomUUID())).isFalse();
    }

    @Test
    void list_filtersByStatus() {
        service.submit(Job.builder().name("job-a").build());
        service.submit(Job.builder().name("job-b").build());

        var pending = service.list(JobStatus.PENDING, null, 0, 10);

        assertThat(pending).hasSize(2);
        assertThat(pending).allMatch(j -> j.status() == JobStatus.PENDING);
    }

    @Test
    void list_sortsByPriorityThenSubmissionTime() {
        service.submit(Job.builder().name("low").priority(JobPriority.LOW).build());
        service.submit(Job.builder().name("high").priority(JobPriority.HIGH).build());
        service.submit(Job.builder().name("critical").priority(JobPriority.CRITICAL).build());

        var jobs = service.list(null, null, 0, 10);

        assertThat(jobs.get(0).priority()).isEqualTo(JobPriority.CRITICAL);
        assertThat(jobs.get(1).priority()).isEqualTo(JobPriority.HIGH);
        assertThat(jobs.get(2).priority()).isEqualTo(JobPriority.LOW);
    }
}
