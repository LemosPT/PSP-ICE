#!/usr/bin/env python3
"""Stream test PCM audio to a PSP-ICE receiver over UDP."""

import argparse
import math
import socket
import struct
import time

from protocol import CHANNEL_AUDIO, MSG_AUDIO_DATA, pack_message

SAMPLE_RATE = 44_100
CHANNELS = 2
FRAMES_PER_PACKET = 256
SAMPLE_FORMAT = struct.Struct("<h")
PACKET_DURATION = FRAMES_PER_PACKET / SAMPLE_RATE


def make_pcm_packet(phase: float, frequency: float, amplitude: float) -> tuple[bytes, float]:
    """Generate one little-endian signed 16-bit stereo PCM packet."""
    samples = bytearray(FRAMES_PER_PACKET * CHANNELS * SAMPLE_FORMAT.size)
    phase_step = 2.0 * math.pi * frequency / SAMPLE_RATE

    for frame in range(FRAMES_PER_PACKET):
        sample = int(math.sin(phase) * amplitude)
        struct.pack_into("<hh", samples, frame * 4, sample, sample)
        phase += phase_step
        if phase >= 2.0 * math.pi:
            phase -= 2.0 * math.pi

    return bytes(samples), phase


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("host", help="PSP's IPv4 address, e.g. 192.168.1.84")
    parser.add_argument("--port", type=int, default=39000)
    parser.add_argument("--frequency", type=float, default=440.0)
    parser.add_argument("--amplitude", type=int, default=5000,
                        help="PCM amplitude (default: 5000; keep volume low)")
    parser.add_argument("--seconds", type=float, default=15.0)
    args = parser.parse_args()

    if not 0 < args.frequency < SAMPLE_RATE / 2:
        parser.error("frequency must be between 0 and the Nyquist frequency")
    if not 0 < args.amplitude <= 12000:
        parser.error("amplitude must be between 1 and 12000")
    if args.seconds <= 0:
        parser.error("seconds must be positive")

    total_packets = math.ceil(args.seconds / PACKET_DURATION)
    phase = 0.0
    sequence = 0
    deadline = time.monotonic()

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        destination = (args.host, args.port)
        print(
            f"Streaming {args.frequency:g} Hz PCM to {args.host}:{args.port} "
            f"({SAMPLE_RATE} Hz, stereo, 16-bit)"
        )
        print("Start with the PSP/car volume low.")

        for _ in range(total_packets):
            pcm, phase = make_pcm_packet(phase, args.frequency, args.amplitude)
            datagram = pack_message(
                CHANNEL_AUDIO, MSG_AUDIO_DATA, sequence, pcm
            )
            sock.sendto(datagram, destination)
            sequence = (sequence + 1) & 0xFFFFFFFF

            deadline += PACKET_DURATION
            delay = deadline - time.monotonic()
            if delay > 0:
                time.sleep(delay)

    print(f"Finished: sent {total_packets} audio packets.")


if __name__ == "__main__":
    main()
