"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
class ServerOptions {
    constructor(json) {
        const parsed = JSON.parse(json);
        this.url = parsed["url"];
    }
}
exports.ServerOptions = ServerOptions;
//# sourceMappingURL=config.js.map