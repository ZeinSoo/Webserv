#!/usr/bin/env python3
import sys

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")
sys.stdout.write("<html>\r\n")
sys.stdout.write("<head><title>CGI Test</title></head>\r\n")
sys.stdout.write("<body>\r\n")
sys.stdout.write("<h1>Python CGI is working!</h1>\r\n")
sys.stdout.write("</body>\r\n")
sys.stdout.write("</html>\r\n")
