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
signature to all OTA files. [pr893]
(https://github.com/SmingHub/Sming/pull/893) provides hooks to the OTA
functionality to allow checking of such signatures.

A proven method for this is, for example, ECDSA in conjunction with
SHA-256. For both steps libraries are available (micro-ecc and Arduino
Cryptosuite).

To use it, you can subclass rBootHttpUpdate like this:

::

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
   class MyUpdate : public rBootHttpUpdate {

   protected:
       virtual void writeInit();
       virtual bool writeFlash(const u8 *data, u16 size);
       virtual bool writeEnd();
       
   private:
       Sha256 *sha256;
       u8      hdr_len;
       MyHdr   hdr;
   };

   //-----------------------------------------------------------------------------
   void MyUpdate::writeInit() {
       rBootHttpUpdate::writeInit();
       sha256  = new Sha256;
       hdr_len = 0;
   }

   bool MyUpdate::writeFlash(const u8 *data, u16 size) {
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
       return rBootHttpUpdate::writeFlash(data, size);
   }

   bool MyUpdate::writeEnd() {
       if (!rBootHttpUpdate::writeEnd()) return 0;

       u8 hash[SHA256_BLOCK_SIZE];
       sha256->final( hash );

       bool sig_ok = /* add signature check here */;
       if (!sig_ok) {
           debugf("wrong signature");
           return 0;
       }
       return 1;
   }
