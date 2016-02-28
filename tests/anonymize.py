import fileinput
import md5
import base64

for line in fileinput.input():
   line = line.strip()
   if line.startswith('SUMMARY:'):
      parts = line.split(':')
      parts[1] = base64.b64encode(md5.md5(parts[1]).digest())
      print(':'.join(parts))
   else:
      print(line)
