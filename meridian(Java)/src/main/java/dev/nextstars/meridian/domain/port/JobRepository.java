package dev.nextstars.meridian.domain.port;

import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface JobRepository {

    Job save(Job job);

    Optional<Job> findById(UUID id);

    List<Job> findAll(JobStatus status, JobPriority priority, int page, int size);

    List<Job> findPending(int limit);

    long count(JobStatus status);

    boolean delete(UUID id);
}
