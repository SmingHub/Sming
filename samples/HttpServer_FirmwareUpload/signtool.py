#!/usr/bin/env python

import argparse
import sys, os
import struct
import codecs
# image signing via libsodium
try:
    import nacl.bindings
    import nacl.encoding
    import nacl.signing
except:
    sys.stderr.write("""\
It seems you have not installed libsodium bindings for python.
Run 'make python-requirements' to install the necessary files using pip (recommended for Windows users).
Linux users are advised to use their distribution's package manager instead. Search for a package 
named 'python-nacl', 'python2-pynacl' or something similar.
""")
    sys.exit(2)
    
    
def parse_arguments():
    parser = argparse.ArgumentParser(description="Generate signed firmware update image")
    parser.add_argument('--genkey', dest='genkey', action='store_true', default=False, help='Generate new signing key')
    parser.add_argument('--keyfile', dest='keyfile', help='Path to signing key file (output if \'--genkey\' is given, otherwise input).')
    parser.add_argument('--pubkey-header', dest='pubkey_header', default='', help='Generate header file with verification key (public key)')
    parser.add_argument('--rom', dest='rom', default='', help='ROM image file with load address, e.g. address=path/to/image')
    parser.add_argument('--out', dest='out', default='', help='Path to signed firmware update file.')
    return parser.parse_args()

def write_pubkey_header(pk, filepath):
    with open(filepath, 'w') as header:
        header.write('''\
#ifndef FIRMWARE_VERIFICATION_KEY_H
#define FIRMWARE_VERIFICATION_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

static const uint8_t firmwareVerificationKey[''' + str(len(pk)) + '''] = {
    ''' + ', '.join('0x%02X' % (b if isinstance(b, int) else ord(b)) for b in pk) + '''
};
#ifdef __cplusplus
}
#endif
#endif // FIRMWARE_VERIFICATION_KEY_H
''')
    print("Verification key written to C header file '%s'\n" % filepath)

def parse_address(string):
    try:
        if string.startswith('0x') or string.startswith('0X'):
            return int(string, 16)
        else:
            return int(string, 10)
    except:
        sys.stderr.write(string + ' is not a valid start address\n')
        raise
    
def load_rom_image(address_file_pair):
    [address, filepath] = address_file_pair.split('=', 1)
    address = parse_address(address)
    try:
        print('Read image file "' + filepath + '"...')
        with open(filepath, 'rb') as f:
            image_content = f.read()
    except:
        sys.stderr.write('Failed to read "' + filepath + '"\n')
        raise

    return (address, image_content)
    
def make_signed_image(address, image, sk, pk = None):
    # signed image format:
    # - 4 byte magic
    # - 4 byte load address of ROM image
    # - 64 byte signature
    # - ROM image content (variable length)
    
    magic = 0xf01af02a
    
    signed_image = struct.pack('II', magic, address)
    
    sig_state = nacl.bindings.crypto_sign_ed25519ph_state()
    nacl.bindings.crypto_sign_ed25519ph_update(sig_state, image)
    signature = nacl.bindings.crypto_sign_ed25519ph_final_create(sig_state, sk)
    print("OTA signature (EdDSA25519ph): " + codecs.encode(signature, 'hex').decode('ascii'))
    
    if pk is not None:
        # test signature verification
        print("Verification key: " + codecs.encode(pk, 'hex').decode('ascii'))
        verify_state = nacl.bindings.crypto_sign_ed25519ph_state()
        nacl.bindings.crypto_sign_ed25519ph_update(verify_state, image)
        nacl.bindings.crypto_sign_ed25519ph_final_verify(verify_state, signature, pk)
    
    signed_image += signature
    signed_image += image
    
    return signed_image

    
def main():
    args = parse_arguments()
    
    if args.genkey:
        print('Generate signing key...')
        sk_seed = nacl.bindings.randombytes(nacl.bindings.crypto_sign_SEEDBYTES)
        
        if args.keyfile:                
            with open(args.keyfile, 'wb') as privkeyfile:
                privkeyfile.write(codecs.encode(sk_seed, 'hex'))
            print("Private signing key written to '%s'." % args.keyfile)
        else:
            sys.stderr.write("Warning: Not output file for generated key is given ('--keyfile'). Generated key will be lost.\n")
    elif args.keyfile:
        print("Read signing key from '%s'" % args.keyfile)
        with open(args.keyfile, 'rb') as keyfile:
            sk_seed = codecs.decode(keyfile.read(), 'hex')
    
    else:
        sys.exit("Neither '--genkey' nor '--keyfile' is given. No signing key available.\n");
    
    (pk, sk) = nacl.bindings.crypto_sign_seed_keypair(sk_seed)
        
    if args.pubkey_header:
        write_pubkey_header(pk, args.pubkey_header)
    
    if args.out:
        if not args.rom:
            sys.exit('No Rom image given. Cannot generate signed update file.')
        
        address, image = load_rom_image(args.rom)
        signed_image = make_signed_image(address, image, sk, pk)

        try:
            print('Write signed update file to "' + args.out + '"')
            with open(args.out, 'wb') as f:
                f.write(signed_image)
        except:
            sys.stderr.write('Failed to write to "' + args.out + '"\n')
    else:
        if args.rom:
            sys.exit('Output file (\'-o\') missing.\n')

if __name__ == "__main__":
    main()
    
