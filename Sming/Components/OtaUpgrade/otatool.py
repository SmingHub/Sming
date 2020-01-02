import argparse
import sys, os, shutil
from datetime import datetime, timedelta
import struct
import codecs
import string

# image signing via libsodium
def import_nacl():
    try:
        global nacl
        nacl = __import__('nacl.bindings', globals(), locals())
        nacl = __import__('nacl.encoding', globals(), locals())
        nacl = __import__('nacl.signing', globals(), locals())
    except:
        sys.stderr.write("""\
    It seems you have not installed libsodium bindings for python.
    Run 'make python-requirements' (!to be implemented!) to install the necessary files using pip (recommended for Windows users).
    Linux users are advised to use their distribution's package manager instead. Search for a package 
    named 'python-nacl' or similar.
    """)
        sys.exit(2)

MAGIC_UNSIGNED = 0xf01af020
MAGIC_SIGNED = 0xf01af02a

def load_keys(keyfilepath):
    try:
        with open(keyfilepath, 'rb') as keyfile:
            sk_seed = codecs.decode(keyfile.read(), 'hex')
    except:
        sys.stderr.write('Could not read signing key from %s\n', keyfilepath)
        raise
    
    return nacl.bindings.crypto_sign_seed_keypair(sk_seed)

def genkey(args):
    print('Generate signing key...')
    import_nacl()
    
    sk_seed = nacl.bindings.randombytes(nacl.bindings.crypto_sign_SEEDBYTES)
    if os.path.exists(args.output):
        backup = args.output + datetime.now().strftime('.%Y%m%d_%H%M%S.bak')
        shutil.copy(args.output, backup) # copy content and permissions
        sys.stderr.write('Warning: Existing key file backed up as %s\n' % backup)

    with open(args.output, 'wb') as privkeyfile:
        privkeyfile.write(codecs.encode(sk_seed, 'hex'))
    os.chmod(args.output, 0o600) # private key should only be readable by owner
    print("Private signing key written to '%s'." % args.output)

def make_key_source(args):
    import_nacl()
    pk, _ = load_keys(args.key)
    pk_bytestring = ', '.join('0x%02X' % (b if isinstance(b, int) else ord(b)) for b in pk)

    with open(args.output, 'w') as source:
        source.write('''\
#include <FakePgmSpace.h>

const uint8_t OTAUpgrade_PublicKey_P[] PROGMEM = {%s};
''' % pk_bytestring)


def make_rom_image(address, filepath):
    try:
        with open(filepath, 'rb') as f:
            image_content = f.read()
    except:
        sys.stderr.write('Failed to read %s\n' % filepath)
        raise

    image_header = struct.pack('<II', address, len(image_content))
    return image_header + image_content

def make_ota_file(args):
    signed = (not args.key is None)
    if signed:
        import_nacl()

    timestamp = int((datetime.now() - datetime(1900, 1, 1)).total_seconds() * 1000)

    assert(len(args.roms) < 256)

    ota = struct.pack('<IQBxxx',
        MAGIC_SIGNED if signed else MAGIC_UNSIGNED,
        timestamp,
        len(args.roms),
    )

    for (address, filepath) in args.roms:
        ota += make_rom_image(address, filepath)
    
    if signed:
        _, sk = load_keys(args.key)
        sig_state = nacl.bindings.crypto_sign_ed25519ph_state()
        nacl.bindings.crypto_sign_ed25519ph_update(sig_state, ota)
        signature = nacl.bindings.crypto_sign_ed25519ph_final_create(sig_state, sk)
        assert(len(signature) == 64)
        ota += signature
    else:
        ota += bytes(64)

    try:
        with open(args.output, 'wb') as f:
            f.write(ota)
    except:
        sys.stderr.write('Failed to write OTA upgrade file %s\n', args.output)
        raise
    print("OTA upgrade file written to %s" % args.output)


def upload_http_post(args):
    print('Uploading firmware...')
    import random
    try:
        # Python 3
        from urllib.request import Request, urlopen
        from urllib.error import HTTPError
    except ImportError:
        # Python 2
        from urllib2 import Request, urlopen, HTTPError
    
    try:
        with open(args.file, 'rb') as f:
            ota_file_content = f.read()
    except:
        sys.stderr.write('Failed to read OTA upgrade file %s\n', args.file)
    
    boundary = ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(50))
    body = ('--' + boundary + '\r\n' \
        + 'Content-Disposition: form-data; name="%s"; filename="%s"\r\n' % (args.field, os.path.basename(args.file)) \
        + 'Content-Type: application/octet-stream\r\n' \
        + '\r\n').encode('ascii') \
        + ota_file_content \
        + ('\r\n--' + boundary + '--').encode('ascii')
    
    headers =  {'content-type': 'multipart/form-data; boundary=' + boundary }

    if not args.url.startswith('http'):
        args.url = 'http://' + args.url
    request = Request(args.url, data=body, headers=headers)
    try:
        response = urlopen(request)
    except HTTPError as e:
        print(e)
        response = e.read()
        print(response.decode('utf-8') if isinstance(response, bytes) else response)
        sys.exit('OTA firmware upload failed')

    print('OTA firmware upload finished')

def make_parser():
    parser = argparse.ArgumentParser(description='Utility for generating OTA upgrade images')
    parser.set_defaults(func=None)
    subparsers = parser.add_subparsers()
    
    genkey_parser = subparsers.add_parser('genkey', help='Generate a new private signing key')
    genkey_parser.add_argument('-o', '--output', required=True, help='Output location of generated key')
    genkey_parser.set_defaults(func=genkey)

    mksource_parser = subparsers.add_parser('mksource', help='Generate cpp file from public signing key (for use with class OtaUpgradeStream)')
    mksource_parser.add_argument('-o', '--output', required=True, help='Output location of generated header file')
    mksource_parser.add_argument('-k', '--key', required=True, help='Input file containing cryptographic key')
    mksource_parser.set_defaults(func=make_key_source)

    mkota_parser = subparsers.add_parser('mkfile', help='Generate OTA upgrade file from ROM images')
    mkota_parser.add_argument('-o', '--output', required=True, help='Output location of OTA upgrade file')
    mkota_parser.add_argument('-k', '--key', help='Input file containing private key for signing the generated OTA file. If omitted, the created OTA file will not be signed.')

    def get_address(string):
        try:
            if string.startswith('0x') or string.startswith('0X'):
                addr = int(string, 16)
            else:
                addr = int(string, 10)
        except:
            raise argparse.ArgumentTypeError('"%s" is not a valid address. Decimal or Hexadecimal (0x...) value expected.' % string)
        if addr < 0:
            raise argparse.ArgumentTypeError('Negative address "%s" not allowed.' % string)
        return addr

    def romspec(spec):
        parts = spec.split('@')
        if len(parts) == 2:
            file, address = parts
            if len(file) > 0:
                address = get_address(address)
                return (address, file)
        raise argparse.ArgumentTypeError('Invalid romspec. Expected format: FILE@ADDRESS')
        
    mkota_parser.add_argument('--rom', action='append', dest='roms', required=True, type=romspec, metavar='FILE@ADDRESS', help="Image file and flash offset address of ROM to include in the OTA upgrade file, e.g. 'rom0.bin@0x2000'")
    mkota_parser.set_defaults(func=make_ota_file)

    upload_parser = subparsers.add_parser('upload', help='HTTP POST upload of OTA upgrade image (encoded as multipart/form-data)')
    upload_parser.add_argument('--field', default='firmware', help='Set the field of the form data field')
    upload_parser.add_argument('-u', '--url', required=True, help='Upload Url')
    upload_parser.add_argument('file', metavar='FILE', help='OTA upgrade file to upload')
    upload_parser.set_defaults(func=upload_http_post)

    return parser

if __name__ == "__main__":
    parser = make_parser()
    args = parser.parse_args()
    fn = args.func
    if fn is None:
        parser.print_usage()
    else:
        fn(args)