#!/bin/bash
(
  echo -ne "POST /db/db_file.txt HTTP/1.1\r\n"
  echo -ne "Host: toto:8080\r\n"
  echo -ne "Transfer-Encoding: chunked\r\n"
  echo -ne "\r\n"
  echo -ne "C\r\n"
  echo -ne "DB location!\r\n"
  echo -ne "0\r\n"
  echo -ne "\r\n"
) | nc localhost 8080
