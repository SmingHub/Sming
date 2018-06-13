#include "NexUpload.h"
NexUpload nex_download("nex.tft",10,115200);
void setup() {
  // put your setup code here, to run once:
    nex_download.upload();
}

void loop() {
  // put your main code here, to run repeatedly:
}
