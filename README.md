# Marathon General Visualizer

Webブラウザからアクセスできるマラソンマッチ用のローカル環境で動作する簡易リアルタイムビジュアライザフレームワークです。

HTTP通信が行える限り、どんな言語で書かれたプログラムからでもビジュアライズ用のデータをWebブラウザに送信できます([C++の例](/client-code-examples/cpp/main.cpp))。

A general browser-based simple real-time visualization framework designed for the marathon match purpose, which works in
the local machines([C++ Example](/client-code-examples/cpp/main.cpp)). 

You can use this visualizer with your program written in any language as long as you implement JSON data sending logic via HTTP.

[English version README is available here](README-en.md)

## デモ (Demo)

### JSONデータを送れるようにしたあなたのプログラム ([/client-code-examples/cpp/main.cpp](/client-code-examples/cpp/main.cpp))
![client-side](demo-program.gif)

### ブラウザにリアルタイムでレンダリング
![frontend-side](demo-browser.gif)

---

## 必要なソフトウェア

- npm (Node Package Manager)

## 必要な知識

- HTML / CSS / Javascript / [JSON](https://www.google.com/search?q=JSON)
  / [Canvas](https://www.google.com/search?q=canvas+javascript)
- React
- Typescript
- HTTPに関する基本的な知識

## 使用法

ビジュアライザ(フロントエンド+バックエンド)をインストール&起動するためには以下のコマンドを実行します。

```
git clone git@github.com:kyuridenamida/marathon-general-visualizer.git # CLONE GIT REPOSITORY
cd marathon-general-visualizer # Move to the repository root directory
npm i # INSTALL PACKAGES
npm start
```

フロントエンドにアクセスするためにはhttp://localhost:3000/ をブラウザで開きます。

JSONメッセージをあなたのプログラムからBackendに送信するためにはjsonボディを含んだPOSTリクエストを http://localhost:8888/json/publish に送ります。

# サンプルビジュアライザで試せるリクエストの例

ビジュアライザを起動してhttp://localhost:3000/ にアクセスした状態で下記のコマンドをターミナルで実行してみてください。 (curlコマンドのインストールが必要です。)

### 長方形をレンダリングする

```sh
curl --location --request POST 'http://localhost:8888/json/publish' \
--header 'Content-Type: application/json' \
--data-raw '{
    "type": "draw",
    "rects": [
        {"l": 100, "r": 200, "d": 100, "u": 200},
         {"l": 300,"r": 400,"d": 300,"u": 400}
    ]
}'
```

### 指定した色でキャンバスをリセットする

```sh
curl --location --request POST 'http://localhost:8888/json/publish' \
--header 'Content-Type: application/json' \
--data-raw '{
    "type": "reset",
    "resetColor": "#eeffee"
}'
```

## 仕組み

![How it works](how-it-works.png)

バックエンド([/src-backend/index.ts](/src-backend/index.ts)) はあなたのプログラムからHTTPを通じて任意のJSONメッセージを受取り、それをフロントエンドにそのまま渡します。

あなたのプログラムをHTTPを通じてjsonメッセージを送れるように改良してください([C++の例](/client-code-examples/cpp/main.cpp)) 。 またフロントエンド([src/App.tsx](/src/App.tsx))にそれをビジュアライズするコードを書いてください。

実はバックエンドについて学ぶ必要があるケースはほとんどありません。

## トラブルシューティング

### Q. ホットリロードがブラウザで動きません。

A. 以下のコマンドを試してみてください。

```
echo fs.inotify.max_user_watches=524288 | sudo tee -a /etc/sysctl.conf && sudo sysctl -p
```

### Q. /json/publishにjsonを送信しましたが、フロントエンドで何も起きません。

A. jsonが正しいことを確認してください。文字列リテラルやキーをダブルクオテーション(")なしで送ってしまいがちです。

バックエンドログのエラーメッセージを確認すると何が起きているかがわかるかもしれません。

## Contact

[@kyuridenamida](https://twitter.com/kyuridenamida)

# License

MIT
