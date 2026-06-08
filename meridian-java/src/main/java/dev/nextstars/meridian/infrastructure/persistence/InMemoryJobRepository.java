package dev.nextstars.meridian.infrastructure.persistence;

import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;
import dev.nextstars.meridian.domain.port.JobRepository;
import org.springframework.stereotype.Repository;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

@Repository
public class InMemoryJobRepository implements JobRepository {

    private final ConcurrentHashMap<UUID, Job> store = new ConcurrentHashMap<>();

    @Override
    public Job save(Job job) {
        store.put(job.id(), job);
        return job;
    }

    @Override
    public Optional<Job> findById(UUID id) {
        return Optional.ofNullable(store.get(id));
    }

    @Override
    public List<Job> findAll(JobStatus status, JobPriority priority, int page, int size) {
        return store.values().stream()
                .filter(j -> status == null   || j.status()   == status)
                .filter(j -> priority == null || j.priority() == priority)
                .sorted(Comparator
                        .comparingInt((Job j) -> j.priority().order())
                        .thenComparing(Job::submittedAt))
                .skip((long) page * size)
                .limit(size)
                .collect(Collectors.toList());
    }

    @Override
    public List<Job> findPending(int limit) {
        return store.values().stream()
                .filter(j -> j.status() == JobStatus.PENDING)
                .sorted(Comparator
                        .comparingInt((Job j) -> j.priority().order())
                        .thenComparing(Job::submittedAt))
                .limit(limit)
                .collect(Collectors.toList());
    }

    @Override
    public long count(JobStatus status) {
        if (status == null) return store.size();
        return store.values().stream().filter(j -> j.status() == status).count();
    }

    @Override
    public boolean delete(UUID id) {
        return store.remove(id) != null;
    }
}
