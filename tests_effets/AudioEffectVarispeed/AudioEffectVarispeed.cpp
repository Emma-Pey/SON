#include "AudioEffectVarispeed.h"

AudioEffectVarispeed::AudioEffectVarispeed()
: AudioStream(1, inputQueueArray)
{
  memset(_buf, 0, sizeof(_buf));
  _w = 0;
  _r = 0.0f;
}

void AudioEffectVarispeed::setSpeed(float s)
{
  // limites raisonnables
  if (s < 0.25f) s = 0.25f;
  if (s > 4.0f)  s = 4.0f;
  _targetSpeed = s;
}

void AudioEffectVarispeed::update(void)
{
  audio_block_t* in = receiveReadOnly(0);
  if (!in) return;

  audio_block_t* out = allocate();
  if (!out) {
    release(in);
    return;
  }

  // Lissage (évite les “cracks” quand tu bouges le pot)
  _speed += 0.02f * (_targetSpeed - _speed);

  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

    // 1) écrit l'entrée dans le buffer
    _buf[_w] = in->data[i];
    _w++;
    if (_w >= BUF_LEN) _w = 0;

    // 2) sécurité : garder la lecture derrière l'écriture
    uint32_t r_i = (uint32_t)_r;
    uint32_t dist = (_w + BUF_LEN - r_i) % BUF_LEN;

    if (dist < MIN_LAG) {
      r_i = (_w + BUF_LEN - MIN_LAG) % BUF_LEN;
      _r = (float)r_i;
    } else if (dist > MAX_LAG) {
      r_i = (_w + BUF_LEN - MAX_LAG) % BUF_LEN;
      _r = (float)r_i;
    }

    // 3) lecture interpolée (linéaire)
    r_i = (uint32_t)_r;
    uint32_t r_i2 = r_i + 1;
    if (r_i2 >= BUF_LEN) r_i2 = 0;

    float frac = _r - (float)r_i;
    float s0 = (float)_buf[r_i];
    float s1 = (float)_buf[r_i2];
    float y = s0 + frac * (s1 - s0);

    // 4) avance le pointeur de lecture à la vitesse voulue
    _r += _speed;
    while (_r >= (float)BUF_LEN) _r -= (float)BUF_LEN;

    // 5) sortie
    out->data[i] = (int16_t)y;
  }

  transmit(out);
  release(out);
  release(in);
}
