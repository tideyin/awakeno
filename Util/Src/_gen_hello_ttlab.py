#!/usr/bin/env python3
"""TTS: 'Powered by T T LAB at Rayzerlink' → WAV + hello_ttlab.bin (EOF FA05A55A)."""

import array
import asyncio
import os
import struct
import wave

import edge_tts
import miniaudio

TEXT = "Powered by T T LAB at Rayzerlink."
VOICE = "en-US-JennyNeural"
RATE = "-10%"
TARGET_FS = 16000
# Flash WRDAT max 75KB (EOF marker is not written to flash)
MAX_WAV_BYTES = 75 * 1024
# Extra leading silence so amp soft-start / boot settle does not eat "Powered"
LEAD_SILENCE_MS = 120
EOF_MARKER = bytes([0xFA, 0x05, 0xA5, 0x5A])

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# script lives in Util/Src/ → project root is ../../
if os.path.basename(ROOT) != "awakeno":
    ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

OUT_WAV = os.path.join(os.path.dirname(__file__), "powered_by_ttlab.wav")
OUT_BIN = os.path.join(ROOT, "hello_ttlab.bin")
OUT_SAVED = os.path.join(os.path.dirname(__file__), "wave_data_powered_by_ttlab.c.saved")


async def tts_mp3() -> bytes:
    buf = bytearray()
    comm = edge_tts.Communicate(TEXT, VOICE, rate=RATE)
    async for chunk in comm.stream():
        if chunk["type"] == "audio":
            buf.extend(chunk["data"])
    return bytes(buf)


def trim_silence(pcm: bytes, fs: int, thr: int = 500) -> bytes:
    samples = array.array("h")
    samples.frombytes(pcm)
    n = len(samples)
    if n == 0:
        return pcm
    start = 0
    while start < n and abs(samples[start]) < thr:
        start += 1
    end = n - 1
    while end > start and abs(samples[end]) < thr:
        end -= 1
    # keep ~40 ms of original lead-in (speech attack)
    pad = fs // 25
    start = max(0, start - pad)
    end = min(n - 1, end + pad)
    return samples[start : end + 1].tobytes()


def prepend_silence(pcm: bytes, fs: int, ms: int) -> bytes:
    n = (fs * ms // 1000) * 2  # bytes, 16-bit mono
    return (b"\x00" * n) + pcm


def time_stretch_down(pcm: bytes, factor: float) -> bytes:
    """Drop samples to shorten duration (factor < 1 → shorter)."""
    if factor >= 0.999:
        return pcm
    samples = array.array("h")
    samples.frombytes(pcm)
    out = array.array("h")
    pos = 0.0
    step = 1.0 / factor
    n = len(samples)
    while int(pos) < n:
        out.append(samples[int(pos)])
        pos += step
    return out.tobytes()


def write_wav(path: str, pcm: bytes, fs: int) -> None:
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(fs)
        w.writeframes(pcm)


def wav_file_bytes(pcm: bytes, fs: int) -> bytes:
    data_size = len(pcm)
    riff_size = 36 + data_size
    hdr = struct.pack(
        "<4sI4s4sIHHIIHH4sI",
        b"RIFF",
        riff_size,
        b"WAVE",
        b"fmt ",
        16,
        1,
        1,
        fs,
        fs * 2,
        2,
        16,
        b"data",
        data_size,
    )
    return hdr + pcm


def emit_c_saved(path: str, wav_bytes: bytes) -> None:
    lines = [
        "/* Archived TTS WAV for SPI flash / hello_ttlab.bin */",
        "/* Text: Powered by T T LAB at Rayzerlink. Voice: en-US-JennyNeural */",
        '#include "wave_data.h"',
        "",
        "const char wavetestdata[] = {",
    ]
    for i in range(0, len(wav_bytes), 16):
        chunk = wav_bytes[i : i + 16]
        hexes = ", ".join(f"0x{b:02X}" for b in chunk)
        comma = "," if i + 16 < len(wav_bytes) else ""
        lines.append(f"{hexes}{comma}")
    lines.append("};")
    lines.append("")
    lines.append("const uint32_t wavetestdata_size = sizeof(wavetestdata);")
    lines.append("")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))


def main() -> None:
    print("TTS:", TEXT, VOICE, RATE)
    mp3 = asyncio.run(tts_mp3())
    print("mp3 bytes", len(mp3))
    decoded = miniaudio.decode(
        mp3, nchannels=1, sample_rate=TARGET_FS, output_format=miniaudio.SampleFormat.SIGNED16
    )
    pcm = bytes(decoded.samples)
    print("decoded", decoded.sample_rate, decoded.nchannels, len(pcm))
    pcm = trim_silence(pcm, TARGET_FS)
    print("trimmed pcm", len(pcm))
    pcm = prepend_silence(pcm, TARGET_FS, LEAD_SILENCE_MS)
    print("after lead silence", LEAD_SILENCE_MS, "ms →", len(pcm))

    max_pcm = MAX_WAV_BYTES - 44
    if len(pcm) > max_pcm:
        factor = max_pcm / float(len(pcm))
        # leave a little headroom so we don't clip mid-syllable after stretch
        factor = min(factor * 0.98, 0.98)
        pcm = time_stretch_down(pcm, factor)
        pcm = pcm[: max_pcm - (max_pcm % 2)]
        print("time-stretch factor", round(factor, 3), "pcm", len(pcm))

    write_wav(OUT_WAV, pcm, TARGET_FS)
    wav_bytes = wav_file_bytes(pcm, TARGET_FS)
    assert wav_bytes[:4] == b"RIFF"

    bin_bytes = wav_bytes + EOF_MARKER
    with open(OUT_BIN, "wb") as f:
        f.write(bin_bytes)

    emit_c_saved(OUT_SAVED, wav_bytes)

    print("WAV", OUT_WAV, len(wav_bytes))
    print("BIN", OUT_BIN, len(bin_bytes), "EOF", EOF_MARKER.hex())
    print("SAVED", OUT_SAVED)


if __name__ == "__main__":
    main()
