// Note defines for our song. wantYouGone() should be played uptempo, 100bpm, so
//  each beat is 600ms long. Note lengths derive from that, and rests can be
//  accomplished by a delay of appropriate length.

#define beatLength 600
#define WN      beatLength*4
#define HN      beatLength*2
#define QN      beatLength
#define EN      beatLength/2
#define SN      beatLength/4

void wantYouGone(void)
{
  delay(EN);
  playNote(noteE5, EN);
  playNote(noteA5, EN);
  playNote(noteB5, EN);
  playNote(noteCs6, EN+SN);
  playNote(noteD6, SN);
  playNote(noteCs6, QN);
  
  delay(EN);
  playNote(noteA5, EN);
  playNote(noteA5, EN);
  playNote(noteE5, EN);
  playNote(noteB5, EN);
  playNote(noteA5, EN);
  playNote(noteG5, EN);
  playNote(noteA5, EN);
  
  delay(EN+SN);
  playNote(noteE5, SN);
  playNote(noteA5, EN);
  playNote(noteB5, EN);
  playNote(noteCs6, EN);
  playNote(noteD6, EN);
  playNote(noteCs6, EN);
  playNote(noteB5, EN);
  
  playNote(noteA5, QN);
  playNote(noteB5, EN);
  playNote(noteG5, EN);
  playNote(noteG5, QN);
  delay(HN);
  
  delay(EN);
  playNote(noteE5, EN);
  playNote(noteA5, EN);
  playNote(noteB5, EN);
  playNote(noteCs6,EN+SN);
  playNote(noteD6,SN);
  playNote(noteCs6, QN);
  
  delay(EN);
  playNote(noteA5, EN);
  playNote(noteA5, EN);
  playNote(noteE5, EN);
  playNote(noteB5, EN);
  playNote(noteA5, EN);
  playNote(noteG5, EN);
  playNote(noteA5, EN);
  
  delay(EN);
  playNote(noteE5, EN);
  playNote(noteA5, EN);
  playNote(noteB5, EN);
  playNote(noteCs6, EN);
  playNote(noteD6, EN);
  playNote(noteCs6, EN);
  playNote(noteB5, EN);
  
  playNote(noteA5, QN);
  playNote(noteFs6, QN);
  playNote(noteE6, EN);
  playNote(noteCs6, EN);
  playNote(noteB5, EN);
  playNote(noteAs5, EN+SN);
}
