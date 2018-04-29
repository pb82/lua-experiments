"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const logger_1 = require("./logger");
const program = require("commander");
const axios_1 = require("axios");
const os_1 = require("os");
const path_1 = require("path");
const fs_1 = require("fs");
const config_1 = require("./config");
const HOME = os_1.homedir();
function pingServer(url) {
    url += "/ping";
    return new Promise((resolve, reject) => {
        axios_1.default.get(url)
            .then(resolve)
            .catch(reject);
    });
}
function runAction(url, action, blocking) {
    return new Promise((resolve, reject) => {
        url += ("/actions/" + action);
        blocking && (url += "/block");
        axios_1.default.post(url)
            .then(resolve)
            .catch(reject);
    });
}
function tryReadConfig() {
    const configPath = path_1.join(HOME, ".slcli");
    try {
        const stats = fs_1.statSync(configPath);
        if (stats.isFile()) {
            const contents = fs_1.readFileSync(configPath);
            return new config_1.ServerOptions(contents.toString());
        }
        else {
            return null;
        }
    }
    catch (_a) {
        return null;
    }
}
function readFileAsBase64(path) {
    const contents = fs_1.readFileSync(path);
    return contents.toString("base64");
}
function createAction(url, name, path, timeout, memory) {
    url += "/actions";
    const payload = {
        name,
        timeout: Math.abs(parseInt(timeout)) || 0,
        maxmem: Math.abs(parseInt(memory)) || 0,
        code: readFileAsBase64(path)
    };
    axios_1.default.post(url, payload)
        .then(logSuccess)
        .catch(logError);
}
function updateOrCreateConfig(url) {
    const configPath = path_1.join(HOME, ".slcli");
    const config = { url };
    fs_1.writeFileSync(configPath, JSON.stringify(config));
}
function logSuccess() {
    logger_1.info("Operation successful");
}
function logError(err) {
    logger_1.error(err.toString());
}
function main() {
    const config = tryReadConfig();
    program
        .option("-b, --block", "Wait for response")
        .option("-f, --file <file>", "Wait for response")
        .option("-m, --memory <memory>", "Maximum memory usage")
        .option("-t, --timeout <timeout>", "Action timeout");
    program
        .command("connect <url>")
        .description("Establish server connection")
        .action(url => {
        pingServer(url)
            .then(() => {
            updateOrCreateConfig(url);
            logSuccess();
        })
            .catch(logError);
    });
    program
        .command("ping")
        .description("Ping server for response")
        .action(cmd => {
        if (!config) {
            logger_1.error("Please connect to a server first");
            return;
        }
        pingServer(config.url)
            .then(logSuccess)
            .catch(logError);
    });
    program
        .command("run <action>")
        .description("Run a single action")
        .action(action => {
        runAction(config.url, action, program.block)
            .then(result => {
            logSuccess();
            console.log(result.data);
        })
            .catch(logError);
    });
    program
        .command("create <name>")
        .description("Create a single action")
        .action(name => {
        createAction(config.url, name, program.file, program.timeout, program.memory);
    });
    program
        .command("list")
        .description("List available actions")
        .action(() => {
        const url = config.url + "/actions";
        axios_1.default.get(url)
            .then(result => {
            logger_1.info("Available actions");
            console.log(result.data);
        })
            .catch(logError);
    });
    program.parse(process.argv);
}
main();
//# sourceMappingURL=index.js.map