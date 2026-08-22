/*
 * midi_synth.cpp - Amiga serial MIDI / MT-32 style synthesizer for UAE4All2
 *
 *  - Captures MIDI bytes written by Amiga software to the serial port
 *    (SERDAT $DFF030 and CIAA SDR) through midi_synth_feed_byte().
 *  - 32-voice polyphony (16 MIDI channels x 2 voices) with additive
 *    wavetable instruments, ADSR envelopes, per-channel volume /
 *    expression / pan / pitch bend / sustain pedal.
 *  - General MIDI drum kit on channel 10 (index 9).
 *  - Mixes into the Paula audio buffer at the emulator output rate
 *    (48 kHz on Vita) from midi_synth_mix(), called in the SDL audio
 *    callback thread.
 *
 * Threading: midi_synth_feed_byte() runs on the emulation thread and
 * only pushes bytes into a lock-free ring buffer. All synthesis state
 * lives on the audio callback thread, which drains the ring in
 * midi_synth_mix(). No mutexes are required.
 */

#include "midi_synth.h"

#if defined(__PSP2__) || defined(__SWITCH__)

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MIDI_NUM_CHANNELS 16
#define MIDI_VOICES_PER_CHANNEL 2
#define MIDI_MAX_VOICES (MIDI_NUM_CHANNELS * MIDI_VOICES_PER_CHANNEL)
#define MIDI_WAVE_SIZE 512
#define MIDI_WAVE_MASK (MIDI_WAVE_SIZE - 1)
#define MIDI_NUM_INSTRUMENTS 12

#define MIDI_RING_SIZE 512
#define MIDI_RING_MASK (MIDI_RING_SIZE - 1)

/* ------------------------------------------------------------------ */
/* Lock-free input ring (emulation thread -> audio thread)            */
/* ------------------------------------------------------------------ */

static volatile int s_midi_head = 0;
static volatile int s_midi_tail = 0;
static uint8_t s_midi_ring[MIDI_RING_SIZE];

static volatile int s_enabled = 0;

/* ------------------------------------------------------------------ */
/* Envelope / voice state                                             */
/* ------------------------------------------------------------------ */

enum {
    VOICE_OFF = 0,
    VOICE_ATTACK,
    VOICE_DECAY,
    VOICE_SUSTAIN,
    VOICE_RELEASE
};

typedef struct {
    int active;
    int channel;
    int state;          /* VOICE_* */
    int instrument;     /* wavetable index, melodic voices */
    int drum;           /* drum type 1..N for channel 10, 0 = melodic */
    int note;
    float phase;        /* 0..1 cycle position */
    float step;         /* phase advance per output sample */
    float velocity;     /* 0..1 */
    float volume;       /* channel volume * expression (0..1) */
    float pan_l, pan_r;
    float env;          /* current envelope level 0..1 */
    float attack_step;  /* per-sample envelope steps */
    float decay_step;
    float release_step;
    float sustain_level;
    unsigned int rng;   /* noise generator */
    float noise_phase;  /* drum auxiliary phase (pitch sweeps) */
} MidiVoice;

typedef struct {
    int program;        /* GM program 0..127 */
    int volume;         /* CC7  0..127 */
    int expression;     /* CC11 0..127 */
    int pan;            /* CC10 0..127 (64 = center) */
    int pitch_bend;     /* 0..16383, 8192 = center */
    int sustain;        /* CC64 pedal */
    MidiVoice voices[MIDI_VOICES_PER_CHANNEL];
} MidiChannel;

static MidiChannel s_channels[MIDI_NUM_CHANNELS];

/* Parser state (audio thread only). */
static int s_status = 0;
static int s_data[2];
static int s_data_count = 0;

/* ------------------------------------------------------------------ */
/* Wavetables (additive synthesis)                                    */
/* ------------------------------------------------------------------ */

static float s_waves[MIDI_NUM_INSTRUMENTS][MIDI_WAVE_SIZE];
static int s_waves_built = 0;

typedef struct {
    const float *harmonics; /* pairs: harmonic, amplitude; terminated by 0.0 */
    float sustain;
    float attack_ms;
    float decay_ms;
    float release_ms;
} InstrumentDef;

static const float WAVE_PIANO[] = { 1,1.00f, 2,0.50f, 3,0.33f, 4,0.25f, 5,0.20f, 6,0.16f, 7,0.13f, 8,0.10f, 9,0.07f, 0,0 };
static const float WAVE_STRINGS[] = { 1,1.00f, 2,0.60f, 3,0.40f, 4,0.30f, 5,0.25f, 6,0.20f, 7,0.16f, 8,0.13f, 9,0.11f, 10,0.10f, 0,0 };
static const float WAVE_BRASS[] = { 1,1.00f, 2,0.70f, 3,0.45f, 4,0.25f, 5,0.15f, 6,0.10f, 0,0 };
static const float WAVE_SYNTH[] = { 1,1.00f, 3,0.33f, 5,0.20f, 7,0.14f, 9,0.10f, 0,0 };
static const float WAVE_ORGAN[] = { 1,1.00f, 2,0.80f, 3,0.60f, 4,0.50f, 6,0.33f, 8,0.25f, 0,0 };
static const float WAVE_BASS[] = { 1,1.00f, 2,0.50f, 3,0.25f, 4,0.12f, 0,0 };
static const float WAVE_PAD[] = { 1,1.00f, 2,0.50f, 3,0.33f, 4,0.25f, 5,0.20f, 6,0.16f, 7,0.14f, 8,0.12f, 9,0.11f, 10,0.10f, 0,0 };
static const float WAVE_BELL[] = { 1,1.00f, 2,0.60f, 3,0.30f, 4,0.15f, 5,0.08f, 6,0.05f, 7,0.03f, 0,0 };
static const float WAVE_SAW[] = { 1,1.00f, 2,0.50f, 3,0.33f, 4,0.25f, 5,0.20f, 6,0.16f, 7,0.14f, 8,0.12f, 9,0.11f, 10,0.10f, 11,0.09f, 12,0.08f, 0,0 };
static const float WAVE_FLUTE[] = { 1,1.00f, 2,0.15f, 3,0.08f, 0,0 };
static const float WAVE_GUITAR[] = { 1,1.00f, 2,0.60f, 3,0.30f, 4,0.15f, 5,0.10f, 6,0.07f, 0,0 };

static const InstrumentDef s_instruments[MIDI_NUM_INSTRUMENTS] = {
    { WAVE_PIANO,   0.20f, 2, 90,  220 },  /* 0 piano   */
    { WAVE_STRINGS, 0.80f, 30, 250, 320 },  /* 1 strings */
    { WAVE_BRASS,   0.80f, 20, 180, 260 },  /* 2 brass   */
    { WAVE_SYNTH,   0.70f, 5, 120, 200 },   /* 3 synth lead */
    { WAVE_ORGAN,   0.90f, 10, 100, 160 },  /* 4 organ   */
    { WAVE_BASS,    0.70f, 4, 90,  180 },   /* 5 bass    */
    { WAVE_PAD,     0.85f, 40, 400, 500 },  /* 6 pad     */
    { WAVE_BELL,    0.05f, 2, 120, 900 },   /* 7 bell    */
    { WAVE_SAW,     0.60f, 8, 150, 200 },   /* 8 saw     */
    { WAVE_FLUTE,   0.80f, 25, 200, 220 },  /* 9 flute   */
    { WAVE_GUITAR,  0.45f, 2, 120, 260 },   /* 10 guitar */
    { WAVE_STRINGS, 0.85f, 30, 300, 400 }   /* 11 choir/pad */
};

/* GM program -> instrument index (compact table, 8 per row). */
static const int s_gm_map[128] = {
    0,0,0,0,0,0,0,0, 7,7,7,7,7,7,7,7,      /* 0-15  piano/perc */
    4,4,4,4,4,4,4,4, 10,10,10,10,10,10,10,10, /* 16-31 organ/guitar */
    5,5,5,5,5,5,5,5, 1,1,1,1,1,1,1,1,      /* 32-47 bass/strings */
    1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,      /* 48-63 ensemble/brass */
    9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,      /* 64-79 reed/pipe */
    3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6,      /* 80-95 synth lead/pad */
    6,6,6,6,6,6,6,6, 9,9,9,9,9,9,9,9,      /* 96-111 FX/ethnic */
    7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7       /* 112-127 perc/effects */
};

/* ------------------------------------------------------------------ */
/* Drum types                                                         */
/* ------------------------------------------------------------------ */

enum {
    DRUM_KICK = 1,
    DRUM_SNARE,
    DRUM_HIHAT_CLOSED,
    DRUM_HIHAT_OPEN,
    DRUM_HIHAT_PEDAL,
    DRUM_TOM_LOW,
    DRUM_TOM_MID,
    DRUM_TOM_HIGH,
    DRUM_CRASH,
    DRUM_RIDE,
    DRUM_CLAP,
    DRUM_COWBELL
};

static int note_to_drum(int note)
{
    switch (note) {
        case 35: case 36:               return DRUM_KICK;
        case 38: case 40:               return DRUM_SNARE;
        case 42:                        return DRUM_HIHAT_CLOSED;
        case 44:                        return DRUM_HIHAT_PEDAL;
        case 46:                        return DRUM_HIHAT_OPEN;
        case 41: case 43:               return DRUM_TOM_LOW;
        case 45: case 47:               return DRUM_TOM_MID;
        case 48: case 50:               return DRUM_TOM_HIGH;
        case 49: case 57:               return DRUM_CRASH;
        case 51: case 59:               return DRUM_RIDE;
        case 39:                        return DRUM_CLAP;
        case 56: case 60: case 61: case 62: case 63: case 64:
        case 65: case 66: case 67: case 68: case 69: case 70:
        case 71: case 72: case 73: case 74: case 75: case 76:
        case 77: case 78: case 79: case 80: case 81: case 82:
        case 83: case 84: case 85: case 86: case 87:
                                        return DRUM_COWBELL;
        default:                        return DRUM_HIHAT_CLOSED;
    }
}

/* ------------------------------------------------------------------ */

static float note_frequency(int note)
{
    return 440.0f * powf(2.0f, (float)(note - 69) / 12.0f);
}

static void build_waves(void)
{
    for (int inst = 0; inst < MIDI_NUM_INSTRUMENTS; inst++) {
        float *table = s_waves[inst];
        const float *spec = s_instruments[inst].harmonics;
        for (int i = 0; i < MIDI_WAVE_SIZE; i++) {
            float sample = 0.0f;
            for (int h = 0; spec[h * 2] > 0.0f; h++) {
                float harmonic = spec[h * 2];
                float amp = spec[h * 2 + 1];
                sample += amp * sinf(2.0f * 3.14159265f * harmonic * (float)i / (float)MIDI_WAVE_SIZE);
            }
            table[i] = sample;
        }
        /* Normalize to 0..1 peak. */
        float peak = 0.0001f;
        for (int i = 0; i < MIDI_WAVE_SIZE; i++) {
            float a = fabsf(table[i]);
            if (a > peak) peak = a;
        }
        for (int i = 0; i < MIDI_WAVE_SIZE; i++)
            table[i] /= peak;
    }
    s_waves_built = 1;
}

static void voice_off(MidiVoice *v)
{
    v->active = 0;
    v->state = VOICE_OFF;
    v->env = 0.0f;
    v->drum = 0;
}

static void voice_start(MidiVoice *v, int channel, int instrument, int note,
                        float velocity, float sample_rate)
{
    v->active = 1;
    v->channel = channel;
    v->instrument = instrument;
    v->drum = 0;
    v->note = note;
    v->phase = 0.0f;
    v->step = note_frequency(note) / sample_rate;
    v->velocity = velocity;
    v->rng = (unsigned int)(note * 2654435761u + 12345u);
    v->noise_phase = 0.0f;
    v->env = 0.0f;
    v->state = VOICE_ATTACK;

    const InstrumentDef *def = &s_instruments[instrument];
    v->attack_step = 1.0f / (def->attack_ms * 0.001f * sample_rate);
    if (def->attack_ms <= 0.0f) v->attack_step = 1.0f;
    v->decay_step = 1.0f / (def->decay_ms * 0.001f * sample_rate);
    if (def->decay_ms <= 0.0f) v->decay_step = 1.0f;
    v->release_step = 1.0f / (def->release_ms * 0.001f * sample_rate);
    if (def->release_ms <= 0.0f) v->release_step = 1.0f;
    v->sustain_level = def->sustain;
}

static void voice_start_drum(MidiVoice *v, int channel, int drum, int note,
                             float velocity, float sample_rate)
{
    v->active = 1;
    v->channel = channel;
    v->instrument = 0;
    v->drum = drum;
    v->note = note;
    v->phase = 0.0f;
    v->step = 0.0f;
    v->velocity = velocity;
    v->rng = (unsigned int)(note * 2654435761u + 99991u);
    v->noise_phase = 0.0f;
    v->env = 1.0f;
    v->state = VOICE_DECAY;
    v->attack_step = 1.0f;
    v->sustain_level = 0.0f;

    /* Envelope decay rates per drum type (per sample). */
    switch (drum) {
        case DRUM_KICK:         v->decay_step = 1.0f / (0.30f * sample_rate); break;
        case DRUM_SNARE:        v->decay_step = 1.0f / (0.22f * sample_rate); break;
        case DRUM_HIHAT_CLOSED: v->decay_step = 1.0f / (0.06f * sample_rate); break;
        case DRUM_HIHAT_PEDAL:  v->decay_step = 1.0f / (0.08f * sample_rate); break;
        case DRUM_HIHAT_OPEN:   v->decay_step = 1.0f / (0.40f * sample_rate); break;
        case DRUM_TOM_LOW:      v->decay_step = 1.0f / (0.45f * sample_rate); break;
        case DRUM_TOM_MID:      v->decay_step = 1.0f / (0.40f * sample_rate); break;
        case DRUM_TOM_HIGH:     v->decay_step = 1.0f / (0.35f * sample_rate); break;
        case DRUM_CRASH:        v->decay_step = 1.0f / (1.10f * sample_rate); break;
        case DRUM_RIDE:         v->decay_step = 1.0f / (0.80f * sample_rate); break;
        case DRUM_CLAP:         v->decay_step = 1.0f / (0.18f * sample_rate); break;
        default:                v->decay_step = 1.0f / (0.25f * sample_rate); break;
    }
    v->release_step = v->decay_step;
}

static void voice_release(MidiVoice *v)
{
    if (!v->active) return;
    if (v->state == VOICE_ATTACK || v->state == VOICE_DECAY || v->state == VOICE_SUSTAIN)
        v->state = VOICE_RELEASE;
}

static void note_off(int channel, int note);

static MidiVoice *find_voice(MidiChannel *ch, int note)
{
    /* Reuse a voice already playing this exact note. */
    for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
        MidiVoice *v = &ch->voices[i];
        if (v->active && v->note == note && v->state != VOICE_RELEASE)
            return v;
    }
    /* Free slot. */
    for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
        MidiVoice *v = &ch->voices[i];
        if (!v->active)
            return v;
    }
    /* Steal the oldest (release state first, then sustain). */
    MidiVoice *victim = &ch->voices[0];
    for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
        if (ch->voices[i].state == VOICE_RELEASE) {
            victim = &ch->voices[i];
            break;
        }
    }
    return victim;
}

static void note_on(int channel, int note, int velocity, float sample_rate)
{
    MidiChannel *ch = &s_channels[channel];
    if (velocity == 0) {
        note_off(channel, note);
        return;
    }
    MidiVoice *v = find_voice(ch, note);
    float vel = (float)velocity / 127.0f;
    if (vel < 0.05f) vel = 0.05f;

    if (channel == 9) {
        int drum = note_to_drum(note);
        voice_start_drum(v, channel, drum, note, vel, sample_rate);
    } else {
        int inst = s_gm_map[ch->program & 0x7F];
        voice_start(v, channel, inst, note, vel, sample_rate);
    }

    v->volume = (float)ch->volume * (float)ch->expression / (127.0f * 127.0f);
    if (v->volume < 0.01f) v->volume = 0.01f;
    if (v->volume > 1.0f) v->volume = 1.0f;

    if (channel == 9) {
        v->pan_l = 0.7f;
        v->pan_r = 0.7f;
    } else {
        float pan = (float)(ch->pan - 64) / 64.0f;      /* -1..1 */
        if (pan < -1.0f) pan = -1.0f;
        if (pan > 1.0f) pan = 1.0f;
        v->pan_l = cosf((pan + 1.0f) * 0.785398f);      /* 0..pi/2 */
        v->pan_r = sinf((pan + 1.0f) * 0.785398f);
    }

    /* Apply pitch bend immediately. */
    float bend = (float)(ch->pitch_bend - 8192) / 8192.0f * 2.0f;
    v->step *= powf(2.0f, bend / 12.0f);
}

static void note_off(int channel, int note)
{
    MidiChannel *ch = &s_channels[channel];
    if (ch->sustain) {
        /* Sustain pedal down: keep note; it will be released on CC64=0. */
        return;
    }
    for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
        MidiVoice *v = &ch->voices[i];
        if (v->active && v->note == note && v->state != VOICE_RELEASE)
            voice_release(v);
    }
}

static void control_change(int channel, int cc, int value)
{
    MidiChannel *ch = &s_channels[channel];
    switch (cc) {
        case 7:
            ch->volume = value & 0x7F;
            break;
        case 10:
            ch->pan = value & 0x7F;
            break;
        case 11:
            ch->expression = value & 0x7F;
            break;
        case 64:
            ch->sustain = value & 0x7F;
            if (!ch->sustain) {
                /* Release all sustained notes. */
                for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++)
                    voice_release(&ch->voices[i]);
            }
            break;
        case 120: /* all sound off */
        case 123: /* all notes off */
            for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++)
                voice_release(&ch->voices[i]);
            break;
        default:
            break;
    }
}

static void handle_message(int status, int d0, int d1, float sample_rate)
{
    int channel = status & 0x0F;
    int type = status & 0xF0;
    switch (type) {
        case 0x80: /* note off */
            note_off(channel, d0 & 0x7F);
            break;
        case 0x90: /* note on */
            note_on(channel, d0 & 0x7F, d1 & 0x7F, sample_rate);
            break;
        case 0xA0: /* poly aftertouch - ignored */
        case 0xD0: /* channel pressure - ignored */
            break;
        case 0xB0: /* control change */
            control_change(channel, d0 & 0x7F, d1 & 0x7F);
            break;
        case 0xC0: /* program change */
            s_channels[channel].program = d0 & 0x7F;
            break;
        case 0xE0: /* pitch bend */
            s_channels[channel].pitch_bend = ((d1 & 0x7F) << 7) | (d0 & 0x7F);
            break;
        default:
            break;
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void midi_synth_init(void)
{
    if (!s_waves_built)
        build_waves();
    midi_synth_reset();
}

void midi_synth_reset(void)
{
    s_midi_head = 0;
    s_midi_tail = 0;
    s_status = 0;
    s_data_count = 0;
    for (int c = 0; c < MIDI_NUM_CHANNELS; c++) {
        MidiChannel *ch = &s_channels[c];
        ch->program = 0;
        ch->volume = 100;
        ch->expression = 127;
        ch->pan = 64;
        ch->pitch_bend = 8192;
        ch->sustain = 0;
        for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++)
            voice_off(&ch->voices[i]);
    }
}

void midi_synth_set_enabled(int enabled)
{
    if (enabled && !s_enabled) {
        /* Fresh start: flush any stale queued bytes. */
        s_midi_head = 0;
        s_midi_tail = 0;
        s_status = 0;
        s_data_count = 0;
    }
    if (!enabled)
        midi_synth_all_notes_off();
    s_enabled = enabled ? 1 : 0;
}

int midi_synth_enabled(void)
{
    return s_enabled;
}

void midi_synth_all_notes_off(void)
{
    for (int c = 0; c < MIDI_NUM_CHANNELS; c++) {
        for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++)
            voice_release(&s_channels[c].voices[i]);
    }
}

void midi_synth_feed_byte(uint8_t byte)
{
    if (!s_enabled)
        return;
    int head = s_midi_head;
    if (head - s_midi_tail < MIDI_RING_SIZE) {
        s_midi_ring[head & MIDI_RING_MASK] = byte;
        s_midi_head = head + 1;
    }
}

/* ------------------------------------------------------------------ */
/* Sample generation                                                  */
/* ------------------------------------------------------------------ */

static float table_sample(const float *table, float phase)
{
    float pos = phase * (float)MIDI_WAVE_SIZE;
    int idx = (int)pos & MIDI_WAVE_MASK;
    float frac = pos - floorf(pos);
    return table[idx] * (1.0f - frac) + table[(idx + 1) & MIDI_WAVE_MASK] * frac;
}

static float noise_sample(unsigned int *rng)
{
    *rng = *rng * 1664525u + 1013904223u;
    return ((float)((*rng >> 8) & 0xFFFF) / 32767.5f) - 1.0f;
}

/* Render one output frame for a voice; returns sample (pre-pan, -1..1). */
static float render_voice(MidiVoice *v, float sample_rate)
{
    float out;

    if (v->drum) {
        /* ---- Synthesized drum kit ---- */
        v->env -= v->decay_step;
        if (v->env <= 0.0f) {
            v->env = 0.0f;
            v->active = 0;
            v->state = VOICE_OFF;
            return 0.0f;
        }
        float env = v->env;
        switch (v->drum) {
            case DRUM_KICK: {
                /* Sine with downward pitch sweep. */
                float freq = 100.0f - v->noise_phase * 70.0f;
                if (freq < 35.0f) freq = 35.0f;
                v->phase += freq / sample_rate;
                v->noise_phase += 1.0f / sample_rate;
                out = sinf(2.0f * 3.14159265f * v->phase) * env;
                break;
            }
            case DRUM_SNARE: {
                float tone = sinf(2.0f * 3.14159265f * v->phase) * 0.35f;
                v->phase += 190.0f / sample_rate;
                out = tone * env + noise_sample(&v->rng) * 0.75f * env;
                break;
            }
            case DRUM_HIHAT_CLOSED:
                out = noise_sample(&v->rng) * env;
                break;
            case DRUM_HIHAT_PEDAL:
                out = noise_sample(&v->rng) * env * 0.7f;
                break;
            case DRUM_HIHAT_OPEN:
                out = noise_sample(&v->rng) * env;
                break;
            case DRUM_TOM_LOW:
            case DRUM_TOM_MID:
            case DRUM_TOM_HIGH: {
                float base = (v->drum == DRUM_TOM_LOW) ? 90.0f : (v->drum == DRUM_TOM_MID) ? 130.0f : 180.0f;
                float freq = base + (1.0f - v->env) * 60.0f;
                v->phase += freq / sample_rate;
                out = sinf(2.0f * 3.14159265f * v->phase) * env;
                break;
            }
            case DRUM_CRASH:
            case DRUM_RIDE: {
                float tone = sinf(2.0f * 3.14159265f * v->phase) * 0.25f;
                v->phase += 480.0f / sample_rate;
                out = tone * env * 0.5f + noise_sample(&v->rng) * env;
                break;
            }
            case DRUM_CLAP: {
                /* Two quick noise bursts. */
                float burst = (v->env > 0.6f) ? 1.0f : (v->env > 0.25f ? 0.55f : 0.0f);
                out = noise_sample(&v->rng) * env * burst;
                break;
            }
            default: { /* cowbell and misc percussion */
                float tone = sinf(2.0f * 3.14159265f * v->phase);
                v->phase += 540.0f / sample_rate;
                out = (tone > 0.0f ? 1.0f : -0.6f) * env;
                break;
            }
        }
        return out;
    }

    /* ---- Melodic voice ---- */
    out = table_sample(s_waves[v->instrument], v->phase);
    v->phase += v->step;
    if (v->phase >= 1.0f)
        v->phase -= 1.0f;

    /* Envelope state machine. */
    switch (v->state) {
        case VOICE_ATTACK:
            v->env += v->attack_step;
            if (v->env >= 1.0f) {
                v->env = 1.0f;
                v->state = VOICE_DECAY;
            }
            break;
        case VOICE_DECAY:
            v->env -= v->decay_step;
            if (v->env <= v->sustain_level) {
                v->env = v->sustain_level;
                v->state = VOICE_SUSTAIN;
            }
            break;
        case VOICE_SUSTAIN:
            break;
        case VOICE_RELEASE:
            v->env -= v->release_step;
            if (v->env <= 0.0f) {
                v->env = 0.0f;
                v->active = 0;
                v->state = VOICE_OFF;
            }
            break;
        default:
            break;
    }
    return out * v->env;
}

void midi_synth_mix(int16_t *samples, int frames, int channels, int output_rate)
{
    if (!samples || frames <= 0 || channels <= 0 || output_rate <= 0)
        return;

    if (!s_enabled) {
        /* Discard any queued bytes while disabled and stay silent. */
        s_midi_tail = s_midi_head;
        return;
    }

    float sample_rate = (float)output_rate;

    /* Drain the input ring and parse MIDI messages. */
    int tail = s_midi_tail;
    while (tail != s_midi_head) {
        uint8_t b = s_midi_ring[tail & MIDI_RING_MASK];
        tail++;
        if (b & 0x80) {
            if (b == 0xF0 || b == 0xF7) {
                /* SysEx start/continue: ignore until end byte. */
                s_status = 0;
                s_data_count = 0;
            } else if (b == 0xF7 || b == 0xFF) {
                /* SysEx end / system common: ignore. */
                s_status = 0;
                s_data_count = 0;
            } else if (b >= 0xF8) {
                /* Real-time messages: ignore. */
            } else {
                s_status = b;
                s_data_count = 0;
            }
        } else if (s_status) {
            s_data[s_data_count++] = b;
            int len = (s_status >= 0xC0 && s_status <= 0xDF) ? 1 : 2;
            if (s_data_count >= len) {
                handle_message(s_status, s_data[0], len > 1 ? s_data[1] : 0, sample_rate);
                s_data_count = 0;
            }
        }
    }
    s_midi_tail = tail;

    /* Update per-channel gains from current CC values. */
    for (int c = 0; c < MIDI_NUM_CHANNELS; c++) {
        MidiChannel *ch = &s_channels[c];
        float vol = (float)ch->volume * (float)ch->expression / (127.0f * 127.0f);
        if (vol < 0.01f) vol = 0.01f;
        if (vol > 1.0f) vol = 1.0f;
        for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
            if (ch->voices[i].active)
                ch->voices[i].volume = vol;
        }
    }

    /* Master gain keeps the mix well below Paula headroom. */
    const float master = 0.35f;

    for (int f = 0; f < frames; f++) {
        float left = 0.0f, right = 0.0f;

        for (int c = 0; c < MIDI_NUM_CHANNELS; c++) {
            MidiChannel *ch = &s_channels[c];
            for (int i = 0; i < MIDI_VOICES_PER_CHANNEL; i++) {
                MidiVoice *v = &ch->voices[i];
                if (!v->active) continue;
                float s = render_voice(v, sample_rate);
                if (!v->active) continue; /* voice finished during render */
                float amp = s * v->volume * v->velocity * master;
                left  += amp * v->pan_l;
                right += amp * v->pan_r;
            }
        }

        if (channels == 1) {
            float mixed = (float)samples[f] + (left + right) * 0.5f;
            if (mixed > 32767.0f) mixed = 32767.0f;
            if (mixed < -32768.0f) mixed = -32768.0f;
            samples[f] = (int16_t)mixed;
        } else {
            float l = (float)samples[f * channels] + left;
            float r = (float)samples[f * channels + 1] + right;
            if (l > 32767.0f) l = 32767.0f;
            if (l < -32768.0f) l = -32768.0f;
            if (r > 32767.0f) r = 32767.0f;
            if (r < -32768.0f) r = -32768.0f;
            samples[f * channels] = (int16_t)l;
            samples[f * channels + 1] = (int16_t)r;
        }
    }
}

#endif /* __PSP2__ || __SWITCH__ */
