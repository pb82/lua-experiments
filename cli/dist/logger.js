"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const winston_1 = require("winston");
const LoggerConfig = {
    console: {
        colorize: true,
        enabled: true,
        level: "info",
    },
};
const logger = new winston_1.Logger({
    console: {
        colors: {
            debug: "blue",
            error: "red",
            info: "green",
            warn: "yellow",
        },
    },
});
if (LoggerConfig.console.enabled) {
    logger.add(winston_1.transports.Console, {
        colorize: LoggerConfig.console.colorize,
        level: LoggerConfig.console.level,
    });
}
exports.info = logger.info;
exports.warn = logger.warn;
exports.error = logger.error;
exports.debug = logger.debug;
function setLogLevel(level) {
    logger.transports.console.level = level;
}
exports.setLogLevel = setLogLevel;
//# sourceMappingURL=logger.js.map