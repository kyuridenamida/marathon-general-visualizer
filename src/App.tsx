import React, {useEffect, useState} from 'react';
import {io} from 'socket.io-client';
import './App.css';

const socket = io();

const badPayloadError = (value: never) => {
    throw `Bad payload with unsupported type: ${JSON.stringify(value)}`
}

export interface Point {
    x: number;
    y: number;
}

export interface Rect {
    l: number;
    r: number;
    u: number;
    d: number;
}

export interface DrawPayload {
    type: "draw";
    rects: Rect[];
}

export interface ResetPayload {
    type: "reset";
    resetColor: string;
}

export type Payload = DrawPayload | ResetPayload;

function toCanvasPoint<T>(mouseEvent: React.MouseEvent<T>, context: CanvasRenderingContext2D): Point {
    return {
        x: mouseEvent.clientX - context.canvas.getBoundingClientRect().left,
        y: mouseEvent.clientY - context.canvas.getBoundingClientRect().top
    }
}

function clearCanvas(canvasContext: CanvasRenderingContext2D, color: string) {
    canvasContext.fillStyle = color;
    canvasContext.fillRect(0, 0, canvasContext.canvas.width, canvasContext.canvas.height);
}

const App = () => {
    /*
     フロントエンドの状態の定義。
     このXXXとsetXXXはuseStateを使って定義できる。setXXXを使って状態を変化させるとページが更新できる。
    */

    const [mainCanvasContext, setMainCanvasContext] = useState<CanvasRenderingContext2D | null>(null);
    const [drawPayload, setDrawPayload] = useState<DrawPayload | null>(null);
    const [dataReceivingCount, setDataReceivingCount] = useState<number>(0);
    const [clickedPoint, setClickedPoint] = useState<Point | null>(null);

    useEffect(() => {
        // ページを開いたときに生成されたCanvasのDOMをステートから参照できるようにする
        const canvas = document.getElementById("mainCanvas") as HTMLCanvasElement | null;
        const context = canvas?.getContext("2d") ?? null;
        setMainCanvasContext(context);
        if (context) {
            clearCanvas(context, 'white');
        }
    }, []);  // deps=[]、これはページを開いたときに一度だけ実行することを意味する。


    useEffect(() => {
        // ページを開いたときにWebSocketからデータ受信したときの挙動(コールバック関数)を定義する。
        socket.on("publish", (payload: Payload) => {
            setDataReceivingCount((currentCount) => currentCount + 1);
            if (payload.type == "draw") {
                setDrawPayload(payload);
            } else if (payload.type == "reset") {
                if (mainCanvasContext) {
                    clearCanvas(mainCanvasContext, payload.resetColor);
                }
            } else {
                try {
                    // もしここがコンパイルエラーになるならpayloadTypeに対する挙動が網羅されていません。
                    throw badPayloadError(payload);
                } catch (e) {
                    console.error(e);
                }
            }
        });
    }, [mainCanvasContext]); // deps=[mainCanvasContext] これはmainCanvasContextのステートが変化したときに再度コールバックを登録し直すことを意味する。

    useEffect(() => {
        if (!mainCanvasContext || !drawPayload) return;
        // このコード例ではdrawPayloadを受け取る毎に完全再描画をしている。
        clearCanvas(mainCanvasContext, 'white');
        drawPayload.rects.forEach(
            (rect) => {
                //黒縁の灰色長方形を描画する
                mainCanvasContext.fillStyle = "gray";
                mainCanvasContext.fillRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);
                mainCanvasContext.strokeStyle = "black";
                mainCanvasContext.strokeRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);

                // クリックされたらClickedと長方形付近に表示する
                const containClickedPoint = clickedPoint !== null
                    && rect.l <= clickedPoint.x && clickedPoint.x <= rect.r
                    && rect.d <= clickedPoint.y && clickedPoint.y <= rect.u;

                if (containClickedPoint) {
                    mainCanvasContext.fillStyle = "blue";
                    mainCanvasContext.font = "20px Arial";
                    mainCanvasContext.fillText("Clicked", rect.l, rect.d);
                }
            });
    }, [mainCanvasContext, drawPayload, clickedPoint]);　 // ここは描画に関係するステートを入れる。そのステートのいずれかが書き換わったときに再描画が行われる。

    return (
        <div className="App">
            <div className="App-main">
                <p>
                    Data Receiving Count: {dataReceivingCount}
                </p>
                <p>
                    clickedPoint: {clickedPoint ? `(${clickedPoint.x.toFixed(3)}, ${clickedPoint.y.toFixed(3)})}` : "(Not clicked yet)"}
                </p>
                <canvas width="1000" height="1000" id="mainCanvas"
                        onClick={(e) => {
                            if (!mainCanvasContext) return;
                            const point = toCanvasPoint(e, mainCanvasContext);
                            console.log("Mouse click", point);
                            setClickedPoint(point);
                        }}
                />

            </div>
        </div>
    );
}

export default App;
