// example.worker.js
importScripts("https://cdn.jsdelivr.net/npm/xterm-pty@0.9.4/workerTools.js");

onmessage = (msg) => {
    importScripts(location.origin + "/example-core.js");

    emscriptenHack(new TtyClient(msg.data));
};
