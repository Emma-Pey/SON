#ifndef AudioEffectVarispeed_h_
#define AudioEffectVarispeed_h_

#include <Arduino.h>
#include <AudioStream.h>

class AudioEffectVarispeed : public AudioStream {
public:
  AudioEffectVarispeed();

  // speed = 1.0 normal, >1.0 accélère (et monte la hauteur), <1.0 ralentit
  void setSpeed(float s);
  float getSpeed() const { return _targetSpeed; }

  virtual void update(void) override;

private:
  audio_block_t* inputQueueArray[1];

  static constexpr uint32_t BUF_LEN = 8192;   // 8192 échantillons (~186 ms à 44.1k)
  static constexpr uint32_t MIN_LAG = 256;    // marge sécurité pour ne pas lire trop près de l’écriture
  static constexpr uint32_t MAX_LAG = BUF_LEN - 256;

  int16_t  _buf[BUF_LEN];
  uint32_t _w = 0;         // index écriture
  float    _r = 0.0f;      // position lecture (fractionnaire)

  volatile float _targetSpeed = 1.0f; // consigne (potentiomètre)
  float _speed = 1.0f;               // vitesse lissée (anti-crackles)
};

#endif
