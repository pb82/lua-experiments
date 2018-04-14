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
    value=42,
    test="asdf",
    test2=true,
    test3=false
}
