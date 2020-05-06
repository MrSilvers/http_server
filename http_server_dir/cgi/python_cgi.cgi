#!/usr/bin/env python3
# -*- coding:UTF-8 -*-
import cgi, cgitb
import json

form = cgi.FieldStorage()

something = form.getvalue("something")

print("Cache-Control: max-age=2592000")
print("Connection: keep-alive")
print("Server: Http_server")
print("Content-Type: application/json;text/plain")
print()
data = {"data":something}
print(json.dumps(data))


