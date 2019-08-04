*******************
Signed OTA Updating
*******************

Introduction
============

Deploying embedded devices with (automatic) OTA functionality introduces
new risks to local networks and the whole internet. If an attacker takes
over the update server or runs a MITM attack, he might be able to turn
the devices into zombies.

To prevent this, you can either provide a secure connection between
device and update server (e. g. VPN or TLS) or add a cryptographic
signature to all OTA files. :pull-request:`893` provides hooks to the OTA
functionality to allow checking of such signatures.

A proven method for this is, for example, ECDSA in conjunction with
SHA-256. For both steps libraries are available (micro-ecc and Arduino
Cryptosuite).

To use it, you can subclass RbootOutputStream like this:

.. code-block:: c++

   #define PREFIX_MAGIC    0x54, 0x49, 0x55, 0x53
   #define PREFIX_TYPE     0x00, 0x01
   #define PREFIX_SIZE     sizeof(_my_prefix)
   #define SIGNATURE_SIZE  64

   const u8 _my_prefix[6] = { PREFIX_MAGIC, PREFIX_TYPE };

   typedef struct {
       u8  prefix[PREFIX_SIZE];
       u8  signature[SIGNATURE_SIZE];
   } MyHdr;

   //-----------------------------------------------------------------------------
   class MyStream : public RbootOutputStream {
   public:
      MyStream(uint32_t startAddress, size_t maxLength = 0): RbootOutputStream(startAddress, maxLength)
      {
         // do some initialization if needed.
      }

      size_t write(const uint8_t* data, size_t size) override;
      bool close() override;
      virtual ~MyStream()
      {
        delete sha256;
      }

   protected:
       bool init() override;

   private:
       Sha256 *sha256 = nullptr;
       u8      hdr_len;
       MyHdr   hdr;
   };

   //-----------------------------------------------------------------------------
   bool MyStream::init() {
       RbootOutputStream::init();
       delete sha256;
       sha256  = new Sha256;
       hdr_len = 0;
   }

   size_t MyStream::write(const uint8_t* data, size_t size) {
       //  store header
       u8 missing = sizeof(hdr) - hdr_len;
       if (missing) {
           if (size < missing) missing = size;
           memcpy( &hdr, data, missing );
           size    -= missing;
           data    += missing;
           hdr_len += missing;

           //  check prefix
           if ( hdr_len >= PREFIX_SIZE ) {
               if ( memcmp(hdr.prefix, _my_prefix, PREFIX_SIZE) ) {
                   debugf("invalid prefix");
                   return 0;
               }
           }
       }

       //  update hash
       sha256->update(data, size);

       //  save data
       return RbootOutputStream::write(data, size);
   }

   bool MyStream::close() {
       if (!RbootOutputStream::close()) {
         return false;
       }

       u8 hash[SHA256_BLOCK_SIZE];
       sha256->final( hash );

       bool sig_ok = /* add signature check here */;
       if (!sig_ok) {
           debugf("wrong signature");
           // TODO: if needed delete the block at the startAddress
           return 0;
       }
       return 1;
   }


And then in your application you can use your MyStream with the following setup:

.. code-block:: c++

  RbootHttpUpdater* otaUpdater = new RbootHttpUpdater();

  MyStream* stream = new MyStream(1234); // Replace 1234 with the right start address

  otaUpdater->addItem(ROM_0_URL, new MyStream()); // << the second parameter specifies that your stream will be used to store the data.

  // and/or set a callback (called on failure or success without switching requested)
  otaUpdater->setCallback(OtaUpdate_CallBack);

