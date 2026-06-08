package dev.nextstars.meridian.domain.model;

public enum JobPriority {
    CRITICAL(0),
    HIGH(1),
    NORMAL(2),
    LOW(3);

    private final int order;

    JobPriority(int order) {
        this.order = order;
    }

    public int order() {
        return order;
    }
}
