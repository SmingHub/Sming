import argparse
import sys, os, shutil
from datetime import datetime
import struct
import codecs
import string

# image signing via libsodium
def import_nacl():
    try:
        global nacl
        nacl = __import__('nacl.bindings', globals(), locals())
        nacl = __import__('nacl.encoding', globals(), locals())
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
            seed = codecs.decode(keyfile.read(), 'hex')
    except:
        sys.stderr.write('Could not read signing key from %s\n', keyfilepath)
        raise

    min_bytes = nacl.bindings.crypto_sign_SEEDBYTES + nacl.bindings.crypto_secretstream_xchacha20poly1305_KEYBYTES
    assert len(seed) >= min_bytes, 'Invalid OTA key file %s' % keyfilepath

    pk, sk = nacl.bindings.crypto_sign_seed_keypair(seed[0:nacl.bindings.crypto_sign_SEEDBYTES])
    enc_key_offset = nacl.bindings.crypto_sign_SEEDBYTES
    ek = seed[enc_key_offset:(enc_key_offset + nacl.bindings.crypto_secretstream_xchacha20poly1305_KEYBYTES)]
    return pk, sk, ek

def genkey(args):
    print('Generate OTA security key...')
    import_nacl()

    if os.path.exists(args.output):
        backup = args.output + datetime.now().strftime('.%Y%m%d_%H%M%S.bak')
        shutil.copy(args.output, backup) # copy content and permissions
        sys.stderr.write('Warning: Existing key file backed up as %s\n' % backup)

    seed_length = nacl.bindings.crypto_sign_SEEDBYTES + nacl.bindings.crypto_secretstream_xchacha20poly1305_KEYBYTES
    seed = nacl.bindings.randombytes(seed_length)
    with open(args.output, 'wb') as privkeyfile:
        privkeyfile.write(codecs.encode(seed, 'hex'))
    os.chmod(args.output, 0o600) # private key should only be readable by owner
    print("Private OTA security key written to '%s'." % args.output)

def make_key_source(args):
    import_nacl()
    pk, _, ek = load_keys(args.key)
    bytestring = lambda key: ', '.join('0x%02X' % (b if isinstance(b, int) else ord(b)) for b in key)

    with open(args.output, 'w') as source:
        source.write('#include <sys/pgmspace.h>\n\n')
        if args.signed or not args.encryted:
            source.write('uint8_t OTAUpgrade_SignatureVerificationKey_P[] PROGMEM = {%s};\n' % bytestring(pk))
        if args.encrypted:
            source.write('uint8_t OTAUpgrade_EncryptionKey_P[] PROGMEM = {%s};\n' % bytestring(ek))

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
    has_key = args.key is not None
    if (args.signed or args.encrypted) and not has_key:
        raise ValueError('Encryption/Signing requested, but no key given.')

    if not args.signed and not args.encrypted and has_key:
        args.signed = True # assume signing if a key is given, but neither --signed nor --encrypted

    if args.signed or args.encrypted:
        import_nacl()
        _, sk, ek = load_keys(args.key)

    assert(len(args.roms) < 256)

    magic = MAGIC_SIGNED if args.signed else MAGIC_UNSIGNED
    timestamp = int((datetime.now() - datetime(1900, 1, 1)).total_seconds() * 1000)
    ota = struct.pack('<IQBxxx', magic, timestamp, len(args.roms))

    for (address, filepath) in args.roms:
        ota += make_rom_image(address, filepath)

    if args.signed:
        # calculate and append signature over whole file, including header, such that even the build timestamp cannot be forged
        sig_state = nacl.bindings.crypto_sign_ed25519ph_state()
        nacl.bindings.crypto_sign_ed25519ph_update(sig_state, ota)
        ota += nacl.bindings.crypto_sign_ed25519ph_final_create(sig_state, sk)
    else:
        # use MD5sum as a checksum only (not for security!)
        import hashlib
        ota += hashlib.md5(ota).digest()

    if args.encrypted:
        # Whole file is encrypted using libsodium's crypto_secretstream_... API using the following format:
        # - Header of crypto_secretstream_xchacha20poly1305_HEADERBYTES: used to initialize algorithm
        # - N Messages chunks of:
        #   - 2 Bytes (LE): chunk size - 1 => typical chunk size is 2K, last chunk shorter
        #   - encrypted chunk
        default_chunk_size = 2048 - nacl.bindings.crypto_secretstream_xchacha20poly1305_ABYTES
        assert(default_chunk_size > 0)
        enc_state = nacl.bindings.crypto_secretstream_xchacha20poly1305_state()
        enc_ota = nacl.bindings.crypto_secretstream_xchacha20poly1305_init_push(enc_state, ek)

        offset = 0
        while offset < len(ota):
            ota_chunk = ota[offset:(offset + default_chunk_size)]
            message_len = len(ota_chunk)
            offset += message_len
            tag = nacl.bindings.crypto_secretstream_xchacha20poly1305_TAG_MESSAGE if (offset < len(ota)) \
                else nacl.bindings.crypto_secretstream_xchacha20poly1305_TAG_FINAL
            cipher_text = nacl.bindings.crypto_secretstream_xchacha20poly1305_push(enc_state, ota_chunk, tag=tag)

            cipher_text_len = len(cipher_text)
            assert(cipher_text_len > 0 and cipher_text_len <= 0x10000)
            enc_ota += struct.pack('<H', cipher_text_len - 1)
            enc_ota += cipher_text

        ota = enc_ota

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
    mksource_parser.add_argument('-s', '--signed', action='store_true', default=False, help='Include public key for signature validation')
    mksource_parser.add_argument('-e', '--encrypted', action='store_true', default=False, help='Include decryption key (shared key)')
    mksource_parser.set_defaults(func=make_key_source)

    mkota_parser = subparsers.add_parser('mkfile', help='Generate OTA upgrade file from ROM images')
    mkota_parser.add_argument('-o', '--output', required=True, help='Output location of OTA upgrade file')
    mkota_parser.add_argument('-k', '--key',
        help='Input file containing private key for signing the generated OTA file. If omitted, the created OTA file will not be signed.')
    mkota_parser.add_argument('-s', '--signed', action='store_true', default=False, help='Sign upgrade image')
    mkota_parser.add_argument('-e', '--encrypted', action='store_true', default=False, help='Encrypt ROM images')

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

    mkota_parser.add_argument('--rom', action='append', dest='roms', required=True, type=romspec, metavar='FILE@ADDRESS',
        help="Image file and flash offset address of ROM to include in the OTA upgrade file, e.g. 'rom0.bin@0x2000'")
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