#pragma once

#include "HttpRequest.hpp"

using namespace std;

typedef enum CgiState {
	CGI_PROCESSING,
	CGI_FINISHED,
	CGI_ERROR,
} CgiState;

class CgiHandler{
   private:
	int			_child_status;
	int			_child_pid;
	CgiState	_state;
	int			_error_code;

   public:
	CgiHandler();
	void update();
	const CgiState &getState() const;
	const int &getErrorCode() const;
};
