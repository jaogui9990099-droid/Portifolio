package dev.nextstars.meridian.adapter.rest;

import dev.nextstars.meridian.adapter.rest.dto.ApiResponse;
import dev.nextstars.meridian.adapter.rest.dto.CreateJobRequest;
import dev.nextstars.meridian.adapter.rest.dto.JobResponse;
import dev.nextstars.meridian.domain.model.Job;
import dev.nextstars.meridian.domain.model.JobPriority;
import dev.nextstars.meridian.domain.model.JobStatus;
import dev.nextstars.meridian.domain.port.JobScheduler;
import dev.nextstars.meridian.domain.service.JobService;
import jakarta.validation.Valid;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;
import java.util.UUID;

@RestController
@RequestMapping("/api/v1")
public class JobController {

    private final JobService jobService;

    public JobController(JobService jobService) {
        this.jobService = jobService;
    }

    @PostMapping("/jobs")
    public ResponseEntity<ApiResponse<JobResponse>> submit(@Valid @RequestBody CreateJobRequest req) {
        Job job = Job.builder()
                .name(req.name())
                .payload(req.payload())
                .priority(req.priority() != null ? req.priority() : JobPriority.NORMAL)
                .maxRetries(req.maxRetries() != null ? req.maxRetries() : 3)
                .timeoutSeconds(req.timeoutSeconds() != null ? req.timeoutSeconds() : 300)
                .build();

        Job submitted = jobService.submit(job);
        return ResponseEntity.status(HttpStatus.CREATED)
                .body(ApiResponse.ok(JobResponse.from(submitted)));
    }

    @GetMapping("/jobs/{id}")
    public ResponseEntity<ApiResponse<JobResponse>> getById(@PathVariable UUID id) {
        return jobService.findById(id)
                .map(job -> ResponseEntity.ok(ApiResponse.ok(JobResponse.from(job))))
                .orElseGet(() -> ResponseEntity.status(HttpStatus.NOT_FOUND)
                        .body(ApiResponse.error("Job not found: " + id)));
    }

    @GetMapping("/jobs")
    public ResponseEntity<ApiResponse<List<JobResponse>>> list(
            @RequestParam(required = false) JobStatus status,
            @RequestParam(required = false) JobPriority priority,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        if (size < 1 || size > 100) {
            return ResponseEntity.badRequest()
                    .body(ApiResponse.error("size must be between 1 and 100"));
        }

        List<JobResponse> jobs = jobService.list(status, priority, page, size)
                .stream()
                .map(JobResponse::from)
                .toList();

        return ResponseEntity.ok(ApiResponse.ok(jobs));
    }

    @DeleteMapping("/jobs/{id}")
    public ResponseEntity<ApiResponse<Void>> cancel(@PathVariable UUID id) {
        boolean cancelled = jobService.cancel(id);
        if (!cancelled) {
            return ResponseEntity.status(HttpStatus.CONFLICT)
                    .body(ApiResponse.error("Job cannot be cancelled (not found or already terminal)"));
        }
        return ResponseEntity.ok(ApiResponse.ok(null));
    }

    @GetMapping("/jobs/{id}/executions")
    public ResponseEntity<ApiResponse<List<JobResponse.ExecutionSummary>>> executions(@PathVariable UUID id) {
        return jobService.findById(id)
                .map(job -> {
                    List<JobResponse.ExecutionSummary> history = job.executions().stream()
                            .map(JobResponse.ExecutionSummary::from)
                            .toList();
                    return ResponseEntity.ok(ApiResponse.ok(history));
                })
                .orElseGet(() -> ResponseEntity.status(HttpStatus.NOT_FOUND)
                        .body(ApiResponse.error("Job not found: " + id)));
    }

    @GetMapping("/scheduler/stats")
    public ResponseEntity<ApiResponse<Map<String, Object>>> stats() {
        JobScheduler.SchedulerStats s = jobService.schedulerStats();
        Map<String, Object> data = Map.of(
                "activeWorkers", s.activeWorkers(),
                "poolSize", s.poolSize(),
                "pendingQueueDepth", s.pendingQueueDepth(),
                "totalSubmitted", s.totalSubmitted(),
                "totalCompleted", s.totalCompleted(),
                "totalFailed", s.totalFailed()
        );
        return ResponseEntity.ok(ApiResponse.ok(data));
    }

    @ExceptionHandler(IllegalArgumentException.class)
    public ResponseEntity<ApiResponse<Void>> handleIllegalArgument(IllegalArgumentException e) {
        return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
    }
}
