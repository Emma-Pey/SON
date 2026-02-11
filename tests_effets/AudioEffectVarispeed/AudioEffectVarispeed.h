#ifndef AudioEffectVarispeed_h_
#define AudioEffectVarispeed_h_

#include <Arduino.h>
#include <AudioStream.h>

class AudioEffectVarispeed : public AudioStream {
public:
  AudioEffectVarispeed();

  // speed = 1.0 normal, >1.0 accélère (et monte le pitch), <1.0 ralentit
  void setSpeed(float s);
  float getSpeed() const { return _targetSpeed; }

  virtual void update(void) override;

private:
  audio_block_t* inputQueueArray[1];

  // Buffer circulaire (int16)
  // 16384 samples = 16384/44100 ≈ 0.371 s
  static constexpr uint32_t BUF_LEN = 16384;

  // Garde la lecture derrière l’écriture
  static constexpr uint32_t MIN_LAG = 256;     // ~5.8 ms
  static constexpr uint32_t MAX_LAG = 12288;   // ~279 ms

  int16_t  _buf[BUF_LEN];
  uint32_t _w = 0;        // index écriture
  float    _r = 0.0f;     // position lecture (fractionnaire)

  volatile float _targetSpeed = 1.0f; // consigne (potentiomètre)
  float _speed = 1.0f;               // vitesse lissée (anti-cracks)
};

#endif

