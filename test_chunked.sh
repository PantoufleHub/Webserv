#!/bin/bash
(
  echo -ne "POST /test HTTP/1.1\r\n"
  echo -ne "Host: localhost:8080\r\n"
  echo -ne "Transfer-Encoding: chunked\r\n"
  echo -ne "\r\n"
  echo -ne "5\r\n"
  echo -ne "Hello\r\n"
  echo -ne "6\r\n"
  echo -ne " World\r\n"
  echo -ne "0\r\n"
  echo -ne "\r\n"
)
