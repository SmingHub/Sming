#!/usr/bin/env python
########################################################
#
#  Stack Trace Decoder
#  Author: Slavey Karadzhov <slav@attachix.com>
#
########################################################
import shlex
import select
import subprocess
import sys
import re

def usage():
    print "Usage: \n\t%s <file.elf> [<error-stack.log>]" % sys.argv[0]
    
def extractAddresses(data):
    m = re.findall("(40[0-2](\d|[a-f]){5})", data)
    if len(m) == 0:
        return m
      
    addresses = []      
    for item in m:
        addresses.append(item[0]) 
            
    return addresses  
    
if __name__ == "__main__":
    if len(sys.argv)  not in range(2,4):
        usage()
        sys.exit(1)
        
    command = "xtensa-lx106-elf-addr2line -aipfC -e '%s' " % sys.argv[1]
    pipe = subprocess.Popen(shlex.split(command), bufsize=1, stdin=subprocess.PIPE)
    
    if len(sys.argv) > 2:
        data = open(sys.argv[2]).read()
        pipe.communicate("\n".join(extractAddresses(data)))
    else:
        while True:
            data = raw_input()
            addresses = extractAddresses(data)
            if len(addresses) == 0:
                continue
            
#             print "[",addresses,"]"
            
            line = "\r\n".join(addresses)+"\r\n"
#             line = line.ljust(125," ")
            
            pipe.stdin.write(line)
            pipe.stdin.flush()
            
