#include "CgiHandler.hpp"

// TEMP HEADERS
#include <cstdlib>
#include <unistd.h>
#include <wait.h>

CgiHandler::CgiHandler() {
	_state = CGI_PROCESSING;
	_error_code = 500;
	cout << "HANDLING" << endl;

	// PARSE REQUEST

	_child_pid = fork();
	if (_child_pid == 0) {
		// extern char **environ;
		// string path = (string("www/cgi-bin") + _request->getPath());
		// char* const argv[] = { const_cast<char*>(path.c_str()), NULL };
		// cout << "execveing path: " << path << endl;
		// execve(path.c_str(), argv, environ);
		sleep(5);
		cout << "Finished sleep" << endl;
		exit(0);
	}
}

void CgiHandler::update() {
	int result;

	result = waitpid(_child_pid, &_child_status, WNOHANG);
	if (result == 0) {
		// parent behaviour (Will be parsing response i think)
	} else if (result == _child_pid) {
		cout << "Finished" << endl;
		_state = CGI_FINISHED;
	}
}

const CgiState &CgiHandler::getState() const {
	return _state;
}

const int &CgiHandler::getErrorCode() const {
	return _error_code;
}
