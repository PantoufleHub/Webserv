#!/bin/bash

{
  echo -ne "POST /post/toto2.txt HTTP/1.1\r\n"
  echo -ne "Host: localhost:8080\r\n"
  echo -ne "Transfer-Encoding: chunked\r\n"
  echo -ne "Content-Type: text/plain\r\n"
  echo -ne "\r\n"
  echo -ne "5\r\nHello\r\n"
  echo -ne "4\r\n World\r\n"
  echo -ne "0\r\n\r\n"
} | nc -q 1 localhost 8080
