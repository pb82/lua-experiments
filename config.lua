-- Configures the console logger
Logger {
    level="info",
    fancy=true,
    timestamp=false
}

-- Nulldb {
    -- This is the default no-persistence option.
    -- Suitable for development. Needs no configuration.
-- }

Mongodb {
    url="mongodb://database",
    port=27017
}

Plugin "skeleton" {
    file="./skeleton.so"
}
