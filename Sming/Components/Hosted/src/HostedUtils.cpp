#include "HostedUtils.h"
#include <Data/Stream/MemoryDataStream.h>

// See: https://iam777.tistory.com/538

bool pbEncodeData(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
  PbData *data = (PbData*) *arg;
  if(data == nullptr) {
	  return false;
  }

  if (!pb_encode_tag_for_field(stream, field)) {
    return false;
  }

  return pb_encode_string(stream, (uint8_t*)data->value, data->length);
}

bool pbDecodeData(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
  uint8_t buffer[1024] = {0};

  /* We could read block-by-block to avoid the large buffer... */
  if (stream->bytes_left > sizeof(buffer) - 1) {
    return false;
  }

  size_t available = stream->bytes_left;
  if (!pb_read(stream, buffer, stream->bytes_left)) {
    return false;
  }


  MemoryDataStream* data = (MemoryDataStream*) *arg;
  if(data == nullptr) {
	  data = new MemoryDataStream();
	  *arg = (void*)data;
  }
  data->write(buffer, available);
  return true;
}
