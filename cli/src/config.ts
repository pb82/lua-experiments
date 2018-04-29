export class ServerOptions {
    readonly url: string;

    constructor(json: string) {
        const parsed = JSON.parse(json);
        this.url = parsed["url"];
    }
}