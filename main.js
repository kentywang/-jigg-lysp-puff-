const port = process.env.PORT || 8000;
const express = require('express');
const path = require('path');
const app = express();

app.use((req, res, next) => {
    res.append('Cross-Origin-Opener-Policy', "same-origin");
    res.append('Cross-Origin-Embedder-Policy', "require-corp");
    next();
});

app.get('/', function(req, res) {
    res.sendFile(path.join(__dirname, '/index.html'));
});

app.get('/example.worker.js', function(req, res) {
    res.sendFile(path.join(__dirname, '/example.worker.js'));
});

app.get('/example-core.js', function(req, res) {
    res.sendFile(path.join(__dirname, '/example-core.js'));
});

app.get('/example-core.wasm', function(req, res) {
    res.sendFile(path.join(__dirname, '/example-core.wasm'));
});

app.listen(port, () => {
    console.log(`Example app listening on port ${port}`)
})
