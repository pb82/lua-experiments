import {info, error} from "./logger";
import * as program from "commander";
import axios from "axios";
import {homedir} from "os";
import {join} from 'path';
import {statSync} from 'fs';

const HOME = homedir();

function readConfig() {
    const configPath = join(HOME, ".slcli");
    try {
        const stats = statSync(configPath);
        return configPath;
    } catch {
        error("You need to connect to the server first");
        return false;
    }
}

function main() {
    if(!readConfig()) {
        return;
    }

    program
        .command("ping")
        .description("Ping server for response")
        .action(cmd => {
            info("Pinging server at localhost");
            axios
                .get("http://localhost:8080/ping").then(() => {
                    info("Success", HOME);
                })
                .catch(err => {
                    error("Ping", err);
                });
        });

    program.parse(process.argv);
}

main();