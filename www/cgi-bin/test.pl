#!/usr/bin/perl

print "Content-Type: text/html\r\n\r\n";

print "<!DOCTYPE html>\n";
print "<html lang=\"en\">\n";
print "<head><meta charset=\"UTF-8\"><title>CGI Test - Perl</title></head>\n";
print "<body>\n";

print "<h1>CGI Test - Perl</h1>\n";
print "<h2>Environment Variables:</h2>\n";
print "<pre>\n";

foreach my $key (sort keys %ENV) {
    print "$key=$ENV{$key}\n";
}

print "</pre>\n";
print "</body>\n";
print "</html>\n";
