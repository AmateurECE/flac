import Module from './.libs/libFLAC.js';
import * as fs from 'fs';
import * as util from 'util';
let flac;

function streamInfoCallback(streamInfo) {
    console.log(streamInfo);
}

function readCallback(byteBuffer) {
    const fd = this.fd;
    try {
        const bytesRead = fs.readSync(fd, byteBuffer, 0, byteBuffer.byteLength,
                                      null);
        if (bytesRead > 0) {
            return {
                bytesRead,
                status: flac.StreamDecoderReadStatus.CONTINUE
            };
        }

        return {
            bytesRead,
            status: flac.StreamDecoderReadStatus.END_OF_STREAM
        };
    } catch (error) {
        console.error(error);
        return {
            bytesRead: 0,
            status: flac.StreamDecoderReadStatus.ABORT
        };
    }
}

function writeCallback(lpcmBuffer) {
    console.log(lpcmBuffer);
    return flac.StreamDecoderWriteStatus.CONTINUE;
}

function errorCallback(status) {
    console.error('Got error status: ' + status);
}

function main(fd) {
    const decoderCallbacks = {readCallback: readCallback.bind({fd}),
                              writeCallback, errorCallback,
                              streamInfoCallback};
    const decoder = flac.StreamDecoder.implement(decoderCallbacks);
    decoder.init();

    // Seek through the file to the end of the metadata
    decoder.process_until_end_of_metadata();

    // Process a single FLAC frame
    decoder.process_single();
}

const open = util.promisify(fs.open);
Module().then(flacModule => {
    flac = flacModule;
    return open(process.argv[2]);
}).then(fd => {
    main(fd);
});
