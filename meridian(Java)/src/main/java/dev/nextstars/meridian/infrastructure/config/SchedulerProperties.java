package dev.nextstars.meridian.infrastructure.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.context.properties.bind.DefaultValue;

@ConfigurationProperties(prefix = "meridian.scheduler")
public record SchedulerProperties(
        @DefaultValue("8") int poolSize,
        @DefaultValue("10000") int maxQueueSize,
        @DefaultValue("300") long defaultTimeoutSeconds,
        @DefaultValue("3") int maxRetries,
        @DefaultValue("2.0") double backoffMultiplier,
        @DefaultValue("60") long backoffMaxSeconds
) {}
