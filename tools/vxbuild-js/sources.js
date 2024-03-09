export class SourceContainer {
    constructor(source, name) {
        this.source = source;
        this.name = name;
    }

    positionToRowAndCol(position) {
        var linesBeforePosition = this.source.substring(0, position).split(/\r?\n/g);

        return {
            row: linesBeforePosition.length,
            col: linesBeforePosition[linesBeforePosition.length - 1].length + 1
        };
    }
}

export class SourceError extends Error {
    constructor(message, sourceContainer = null, location = null) {
        super(message);

        this.sourceContainer = sourceContainer;
        this.location = location;
    }

    log() {
        console.error(`VxBuild source error: ${this.message}`);

        if (this.sourceContainer == null) {
            return;
        }

        var rowAndCol = this.sourceContainer.positionToRowAndCol(this.location);

        console.error(`    in ${this.sourceContainer.name}:${rowAndCol.row}:${rowAndCol.col}`);

        var lineNumber = String(rowAndCol.row).padStart(4);

        console.error(`${lineNumber} | ${this.sourceContainer.source.split(/\r?\n/g)[rowAndCol.row - 1]}`);
        console.error(`${" ".repeat(lineNumber.length)}  ${" ".repeat(rowAndCol.col)}^ here`);
    }
}