-- Configures the console logger
Logger {
    level="info",
    fancy=true,
    timestamp=false
}

Nulldb {
    -- This is the default no-persistence option.
    -- Suitable for development. Needs no configuration.
}

Plugin "skeleton" {
    arr={
        [1]="a",
        [2]="b"
    }
}
