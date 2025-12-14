#!/usr/bin/env php
<?php
// Simple PHP CGI Test - GET
header("Content-Type: text/plain; charset=utf-8");

echo "PHP CGI Test - GET\n";
echo "REQUEST_METHOD: " . getenv("REQUEST_METHOD") . "\n";
echo "QUERY_STRING: " . getenv("QUERY_STRING") . "\n";
echo "SCRIPT_NAME: " . getenv("SCRIPT_NAME") . "\n";
?>

