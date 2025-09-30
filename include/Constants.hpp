#pragma once

#include <map>
#include <string>

// HTTP constants reused by the config parser and future server modules.
static const std::string HTTP_VERSION = "HTTP/1.1";

static const std::string METHOD_GET = "GET";
static const std::string METHOD_POST = "POST";
static const std::string METHOD_DELETE = "DELETE";
static const std::string METHOD_NONE = "none";

static const std::string DEFAULT_UPLOAD_STORE = "/misc/var/uploads/";

const std::map<std::string, int>& getDirectivesMap();
