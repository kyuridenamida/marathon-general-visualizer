#include <iostream>
#include "./httputils.h"

using namespace std;

struct Rect {
    int l;
    int r;
    int d;
    int u;

    static Rect randomRect() {
        auto rect = Rect{rand() % 1000, rand() % 1000, rand() % 1000, rand() % 1000};
        if (rect.l > rect.r) swap(rect.l, rect.r);
        if (rect.d > rect.u) swap(rect.d, rect.u);
        return rect;
    }
};


string toRectJson(Rect rect) {
    return HttpUtils::mapToJson(
            {
                    {"l", HttpUtils::jsonValue(rect.l)},
                    {"r", HttpUtils::jsonValue(rect.r)},
                    {"d", HttpUtils::jsonValue(rect.d)},
                    {"u", HttpUtils::jsonValue(rect.u)},
            }
    );
}

string buildDrawPayload(vector<Rect> rects) {
    using namespace HttpUtils;
    vector<string> jsonRectList;
    for (auto r : rects) {
        jsonRectList.push_back(toRectJson(r));
    }
    return HttpUtils::mapToJson(
            {
                    {"rects", HttpUtils::toList(jsonRectList)},
                    {"type",  HttpUtils::jsonValue("draw")},
            }
    );
}

int main() {
    while (true) {
        int n = rand() % 100 + 1;
        vector<Rect> rects;
        for (int i = 0; i < n; i++) {
            rects.push_back(Rect::randomRect());
        }
        string drawingPayload = buildDrawPayload(rects);
        cout << "Sending drawing payload with " << rects.size() << " rects" << endl;
        HttpUtils::emitJsonToUrl("http://localhost:8888/json/publish", drawingPayload);
    }
}