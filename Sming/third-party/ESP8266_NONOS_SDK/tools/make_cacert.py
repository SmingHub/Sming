# 
# ESPRESSIF MIT License
# 
# Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
# 
# Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
# it is free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the Software is furnished
# to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 

import os


class Cert(object):
    def __init__(self, name, buff):
        self.name = name
        self.len = len(buff)
        self.buff = buff
        pass
    
    def __str__(self):
        out_str = ['\0']*32
        for i in range(len(self.name)):
            out_str[i] = self.name[i]
        out_str = "".join(out_str)
        out_str += str(chr(self.len & 0xFF))
        out_str += str(chr((self.len & 0xFF00) >> 8))
        out_str += self.buff
        return out_str
        pass


def main():
    cert_list = []
    file_list = os.listdir(os.getcwd())
    cert_file_list = []
    for _file in file_list:
        if _file.endswith(".cer"):
            cert_file_list.append(_file)
    print cert_file_list
    for cert_file in cert_file_list:
        with open(cert_file, 'rb') as f:
            buff = f.read()
        cert_list.append(Cert(cert_file, buff))
    with open('esp_ca_cert.bin', 'wb+') as f:
        for _cert in cert_list:
            f.write("%s" % _cert)
    pass
if __name__ == '__main__':
    main()

