# Meridian

Meridian is a small job scheduling service built with Java 21 and Spring Boot.

It exposes a REST API for submitting jobs, listing them by status or priority, cancelling pending work and reading scheduler statistics. The scheduler uses priority ordering, retry configuration and a bounded queue so the service behaves predictably under load.

## Why this project exists

I built Meridian to show backend service structure without hiding the core logic behind framework code. The domain model, repository port and scheduler port are separated from the REST adapter, so the important behavior can be tested without starting the full application.

## Main features

- Submit jobs with a name, payload, priority, retry count and timeout.
- Process jobs through a priority-based scheduler.
- Track job status and execution history.
- Cancel jobs that have not finished yet.
- Read scheduler stats through an API endpoint.
- Validate API input and return consistent response envelopes.

## Tech

- Java 21
- Spring Boot 3
- Maven
- JUnit 5
- Awaitility

## API

Base path: `/api/v1`

| Method | Path | Description |
| --- | --- | --- |
| `POST` | `/jobs` | Submit a job |
| `GET` | `/jobs` | List jobs with optional status/priority filters |
| `GET` | `/jobs/{id}` | Get a job by id |
| `DELETE` | `/jobs/{id}` | Cancel a job |
| `GET` | `/jobs/{id}/executions` | List execution attempts |
| `GET` | `/scheduler/stats` | Read scheduler counters |

Example request:

```bash
curl -X POST http://localhost:8080/api/v1/jobs \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"sync-report\",\"payload\":{\"reportId\":42},\"priority\":\"HIGH\"}"
```

## Run

```bash
mvn spring-boot:run
```

## Test

```bash
mvn test
```

Current local result: `11 tests, 0 failures`.

## Code map

- `domain/model`: job state, priority and execution result objects.
- `domain/service`: submit, list and cancel behavior.
- `domain/port`: repository and scheduler contracts.
- `infrastructure/scheduler`: priority thread pool implementation.
- `adapter/rest`: HTTP controller and DTOs.
