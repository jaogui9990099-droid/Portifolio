package dev.nextstars.meridian.infrastructure.config;

import dev.nextstars.meridian.domain.service.RetryPolicy;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.time.Duration;

@Configuration
public class AppConfig {

    @Bean
    public RetryPolicy retryPolicy(SchedulerProperties props) {
        return new RetryPolicy(props.backoffMultiplier(), Duration.ofSeconds(props.backoffMaxSeconds()));
    }
}
