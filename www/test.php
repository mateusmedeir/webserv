#!/usr/bin/env php
<?php
// CGI Test Script - PHP
header("Content-Type: text/html; charset=utf-8");

echo "<h1>CGI Test - PHP</h1>\n";
echo "<h2>Environment Variables:</h2>\n";
echo "<pre>\n";

echo "REQUEST_METHOD: " . getenv("REQUEST_METHOD") . "\n";
echo "QUERY_STRING: " . getenv("QUERY_STRING") . "\n";
echo "SCRIPT_NAME: " . getenv("SCRIPT_NAME") . "\n";
echo "SERVER_NAME: " . getenv("SERVER_NAME") . "\n";
echo "SERVER_PORT: " . getenv("SERVER_PORT") . "\n";
echo "CONTENT_TYPE: " . getenv("CONTENT_TYPE") . "\n";
echo "CONTENT_LENGTH: " . getenv("CONTENT_LENGTH") . "\n";

echo "\n</pre>\n";

if (getenv("REQUEST_METHOD") == "POST") {
    echo "<h2>POST Data:</h2>\n";
    echo "<pre>\n";
    $body = file_get_contents("php://stdin");
    echo "Body received: " . strlen($body) . " bytes\n";
    echo "Body content: " . htmlspecialchars($body) . "\n";
    echo "</pre>\n";
}

echo "<p>PHP Version: " . phpversion() . "</p>\n";
?>

