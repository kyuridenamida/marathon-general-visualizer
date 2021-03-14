import * as express from "express";

const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);

const port = Number(process.env.PORT || 8888);
app.use(express.json())

interface JsonPublishRequest {
    body: any; // どんな json content でもOK
}

app.post('/json/publish', function (req: JsonPublishRequest, res: any) {
    // console.log(req.body); // 受け取ったデータを表示するためにはコメントアウトするとよい。
    io.sockets.emit("publish", req.body);
    res.send("ok")
});

http.listen(port, () => {
    console.info('Backend started on port: ' + port);
});
