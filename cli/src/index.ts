import {info, error} from "./logger";
import * as program from "commander";
import axios from "axios";
import {homedir} from "os";
import {join} from 'path';
import {statSync, readFileSync, writeFileSync} from 'fs';
import {ServerOptions} from "./config";

const HOME = homedir();

function pingServer(url: string) : Promise<any> {
    url += "/ping";

    return new Promise<boolean>((resolve: (result: any) => void, reject) => {
        axios.get(url)
            .then(resolve)
            .catch(reject);
    });
}

function runAction(url: string, action: string, blocking: boolean, payload: string) : Promise<any> {
    let payloadObj = {};
    if (payload) {
        payloadObj = readFileAsJson(payload);
    }

    return new Promise((resolve: (result: any) => void, reject) => {
        url += ("/actions/" + action);
        blocking && (url += "/block");

        axios.post(url, payloadObj)
            .then(resolve)
            .catch(reject);
    });
}

function tryReadConfig() : ServerOptions {
    const configPath = join(HOME, ".slcli");
    try {
        const stats = statSync(configPath);
        if (stats.isFile()) {
            const contents: Buffer = readFileSync(configPath);
            return new ServerOptions(contents.toString());
        } else {
            return null;
        }
    } catch {
        return null;
    }
}

function readFileAsBase64(path: string) : string {
    const contents: Buffer = readFileSync(path);
    return contents.toString("base64");
}

function readFileAsJson(path: string) : object {
    const contents: Buffer = readFileSync(path);
    return JSON.parse(contents.toString());
}

function createAction(url: string, name: string, path: string, timeout: string, memory: string) {
    url += "/actions";

    const payload = {
        name,
        timeout: Math.abs(parseInt(timeout)) || 0,
        maxmem: Math.abs(parseInt(memory)) || 0,
        code: readFileAsBase64(path)
    };

    axios.post(url, payload)
        .then(logSuccess)
        .catch(logError);
}

function deleteAction(url: string, name: string) {
    url += ("/actions/" + name);

    axios.delete(url)
        .then(logSuccess)
        .catch(logError);
}

function resolveInvocation(url: string, id: string) {
    url += ("/invocation/" + id);

    axios.get(url, {timeout: 5000})
        .then(result => {
            console.log(result.data);
        })
        .catch(logError);
}

function updateOrCreateConfig(url: string) {
    const configPath = join(HOME, ".slcli");
    const config = {url};
    writeFileSync(configPath, JSON.stringify(config));
}

function logSuccess() {
    info("Operation successful");
}

function logError(err: Error) {
    error(err.toString());
}

function main() {
    const config: ServerOptions = tryReadConfig();

    program
        .option("-b, --block", "Wait for response")
        .option("-p, --payload <file>", "Function arguments")
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
                error("Please connect to a server first");
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
            runAction(config.url, action, program.block, program.payload)
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
            if (!config) {
                error("Please connect to a server first");
                return;
            }

            createAction(config.url, name, program.file, program.timeout, program.memory);
        });

    program
        .command("delete <name>")
        .description("Delete a single action")
        .action(name => {
            if (!config) {
                error("Please connect to a server first");
                return;
            }

            deleteAction(config.url, name);
        });

    program
        .command("resolve <id>")
        .description("Resolve incocation")
        .action(id => {
            if (!config) {
                error("Please connect to a server first");
                return;
            }

            resolveInvocation(config.url, id);
        });

    program
        .command("list")
        .description("List available actions")
        .action(() => {
            if (!config) {
                error("Please connect to a server first");
                return;
            }

            const url = config.url + "/actions";
            axios.get(url)
                .then(result => {
                    info("Available actions");
                    console.log(result.data);
                })
                .catch(logError);
        });

    program.parse(process.argv);
}

main();