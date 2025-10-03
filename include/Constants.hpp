#pragma once

#include <map>
#include <string>

using namespace std;

// HTTP constants reused by the config parser and future server modules.
static const string HTTP_VERSION = "HTTP/1.1";

static const string METHOD_GET = "GET";
static const string METHOD_POST = "POST";
static const string METHOD_DELETE = "DELETE";
static const string METHOD_NONE = "none";

const string DEFAULT_UPLOAD_STORE = "/misc/var/uploads/";

const string HEADER_CONTENT_TYPE = "Content-Type";
const string HEADER_CONTENT_LENGTH = "Content-Length";
const string HEADER_CONNECTION = "Connection";
const string HEADER_HOST = "Host";
const string HEADER_ACCEPT = "Accept";

const string TYPE_HTML = "text/html";

// Config file constants
static const std::string DEFAULT_UPLOAD_STORE = "/misc/var/uploads/";

const std::map<std::string, int>& getDirectivesMap();

// Log file constants
const string LOG_FOLDER = "misc/logs/";
const string CONN_LOG_FILE = LOG_FOLDER + "connections.log";
const string REQ_LOG_FILE = LOG_FOLDER + "requests.log";
const string RESP_LOG_FILE = LOG_FOLDER + "responses.log";
const string ERROR_LOG_FILE = LOG_FOLDER + "errors.log";
