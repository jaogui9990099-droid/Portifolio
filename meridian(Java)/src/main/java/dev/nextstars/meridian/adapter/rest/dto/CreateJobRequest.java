package dev.nextstars.meridian.adapter.rest.dto;

import dev.nextstars.meridian.domain.model.JobPriority;
import jakarta.validation.constraints.*;

public record CreateJobRequest(
        @NotBlank(message = "name is required")
        @Size(max = 128, message = "name must not exceed 128 characters")
        String name,

        @Size(max = 65536, message = "payload must not exceed 64 KiB")
        String payload,

        JobPriority priority,

        @Min(value = 0, message = "maxRetries must be >= 0")
        @Max(value = 10, message = "maxRetries must be <= 10")
        Integer maxRetries,

        @Min(value = 1, message = "timeoutSeconds must be >= 1")
        @Max(value = 86400, message = "timeoutSeconds must be <= 86400")
        Long timeoutSeconds
) {}
