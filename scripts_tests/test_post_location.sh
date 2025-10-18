#!/bin/bash
(
  echo -ne "POST /post/another_file.txt HTTP/1.1\r\n"
  echo -ne "Host: toto:8080\r\n"
  echo -ne "Transfer-Encoding: chunked\r\n"
  echo -ne "\r\n"
  echo -ne "E\r\n"
  echo -ne "Post location!\r\n"
  echo -ne "0\r\n"
  echo -ne "\r\n"
) | nc localhost 8080
