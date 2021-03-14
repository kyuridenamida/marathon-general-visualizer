#ifndef MARATHON_GENERAL_VIS_HTTP_UTILS
#define MARATHON_GENERAL_VIS_HTTP_UTILS

#include <sstream>
#include <string>
#include <vector>
#include "./picohttpclient.hpp"

namespace HttpUtils {
    using namespace std;

    void emitJsonToUrl(const string &url, const string &jsonString) {
        PicoHTTPClient::HTTPResponse response = PicoHTTPClient::HTTPClient::request(
                PicoHTTPClient::HTTPClient::POST,
                PicoHTTPClient::URI(url),
                jsonString);
        if (!response.success) {
            cerr << "Failed to send request" << endl;
        }
    }

    string quoted(string key) {
        return "\"" + key + "\"";
    };

    typedef string JsonValue;

    JsonValue jsonValue(string x) {
        return quoted(x);
    }

    JsonValue jsonValue(int x) {
        return to_string(x);
    }

    JsonValue jsonValue(double x) {
        return to_string(x);
    }

    string mapToJson(const map <string, JsonValue> &jsonMap) {
        stringstream ss;
        for (const auto &kv : jsonMap) {
            ss << quoted(kv.first) << ":" << kv.second << ",";
        }

        string content = ss.str();
        content.pop_back(); // 末尾カンマ消す
        return "{" + content + "}";
    }

    string toList(const vector <string> jsonObjectList) {
        stringstream arrayBuilder;
        arrayBuilder << "[";
        for (int i = 0; i < jsonObjectList.size(); i++) {
            if (i != 0) {
                arrayBuilder << ",";
            }
            arrayBuilder << jsonObjectList[i];
        }
        arrayBuilder << "]";
        return arrayBuilder.str();
    }
}
#endif //MARATHON_GENERAL_VIS_HTTP_UTILS
