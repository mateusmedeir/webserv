#Create sh script for CGI testing
#!/bin/bash
printf "Content-Type: text/html\r\n\r\n"
printf "<h1>CGI Test - SH</h1>\n"
printf "<h2>Environment Variables:</h2>\n"
printf "<pre>\n"
env
printf "</pre>\n"