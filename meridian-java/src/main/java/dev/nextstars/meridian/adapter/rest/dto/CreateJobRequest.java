package dev.nextstars.meridian.adapter.rest.dto;

import dev.nextstars.meridian.domain.model.JobPriority;
import jakarta.validation.constraints.*;

public record CreateJobRequest(
        @NotBlank(message = "name is required")
        @Size(max = 128)
        String name,

        @Size(max = 65536)
        String payload,

        JobPriority priority,

        @Min(0) @Max(10)
        Integer maxRetries,

        @Min(1) @Max(86400)
        Long timeoutSeconds
) {}
