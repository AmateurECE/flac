// import * as fs from 'fs';

// async function main() {
//     const buffer = fs.readFileSync('./a.out.wasm');
//     var imports = {env: {}, imports: {}, wasi_snapshot_preview1: {}};
//     const lib = await WebAssembly.instantiate(new Uint8Array(buffer),
//                                               imports)
//           .then(res => res.instance.exports)
//           .catch(error => console.error(error));
// }

// main();

import Module from './.libs/libFLAC++.js';
let flac;

function main() {
    const ConcreteDecoder = flac.StreamDecoder.extend("StreamDecoder", {
        // Empty implementations, for now.
        readCallback: function(byteBuffer) {
            console.log('Read was called!');
            return {
                bytesRead: 0,
                status: flac.StreamDecoderReadStatus.ABORT
            };
        },

        writeCallback: function(lpcmBuffer) {
            console.log('Write was called!');
            return 0;
        },

        errorCallback: function(status) {
            console.log('Got error status: ' + status);
        }
    });
    
    const decoder = new ConcreteDecoder();
    decoder.init();
    decoder.process_until_end_of_metadata();
}

Module().then(flacModule => {
    flac = flacModule;
    main();
});
