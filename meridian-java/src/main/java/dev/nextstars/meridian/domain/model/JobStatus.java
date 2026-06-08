package dev.nextstars.meridian.domain.model;

public enum JobStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED,
    DEAD_LETTERED;

    public boolean isTerminal() {
        return switch (this) {
            case COMPLETED, FAILED, CANCELLED, DEAD_LETTERED -> true;
            default -> false;
        };
    }
}
