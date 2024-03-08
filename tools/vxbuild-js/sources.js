export class SourceContainer {
    constructor(source, name) {
        this.source = source;
        this.name = name;
    }
}

export class SourceError extends Error {
    constructor(message, location = null) {
        super(message);

        this.location = location;
    }
}