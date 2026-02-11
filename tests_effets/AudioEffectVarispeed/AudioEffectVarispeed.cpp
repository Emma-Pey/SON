#include "AudioEffectVarispeed.h"

AudioEffectVarispeed::AudioEffectVarispeed()
: AudioStream(1, inputQueueArray)
{
  memset(_buf, 0, sizeof(_buf));
  _w = 0;

  // On démarre la lecture "loin derrière" l’écriture pour avoir du stock
  _r = (float)((BUF_LEN + _w - MAX_LAG) % BUF_LEN);
}

void AudioEffectVarispeed::setSpeed(float s)
{
  // Limites raisonnables
  if (s < 0.5f) s = 0.5f;
  if (s > 2.5f) s = 2.5f;
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

  // Lissage (évite clics quand tu bouges le pot)
  _speed += 0.02f * (_targetSpeed - _speed);

  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

    // 1) Écriture dans le buffer
    _buf[_w] = in->data[i];
    _w++;
    if (_w >= BUF_LEN) _w = 0;

    // 2) Sécurité : garder une distance (lag) entre écriture et lecture
    uint32_t r_i = (uint32_t)_r;
    uint32_t lag = (_w + BUF_LEN - r_i) % BUF_LEN;

    // Si trop près : on recule la lecture loin derrière (sinon speed>1 est "annulé")
    if (lag < MIN_LAG) {
      uint32_t new_r = (_w + BUF_LEN - MAX_LAG) % BUF_LEN;
      _r = (float)new_r;
    }
    // Si trop loin (ex: speed<1 longtemps) : on rapproche pour limiter la latence
    else if (lag > MAX_LAG) {
      uint32_t new_r = (_w + BUF_LEN - MIN_LAG) % BUF_LEN;
      _r = (float)new_r;
    }

    // 3) Lecture interpolée (linéaire) à position fractionnaire _r
    r_i = (uint32_t)_r;
    uint32_t r_i2 = r_i + 1;
    if (r_i2 >= BUF_LEN) r_i2 = 0;

    float frac = _r - (float)r_i;
    float s0 = (float)_buf[r_i];
    float s1 = (float)_buf[r_i2];
    float y  = s0 + frac * (s1 - s0);

    // 4) Avancer le pointeur de lecture selon la vitesse
    _r += _speed;
    while (_r >= (float)BUF_LEN) _r -= (float)BUF_LEN;

    // 5) Sortie
    out->data[i] = (int16_t)y;
  }

  transmit(out);
  release(out);
  release(in);
}

