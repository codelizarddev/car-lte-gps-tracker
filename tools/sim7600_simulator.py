#!/usr/bin/env python3
"""
sim7600_simulator.py — SIM7600 AT command modem simulator

Creates a virtual serial port pair and responds to AT commands
exactly as a real SIM7600 would. Useful for testing firmware
logic without physical hardware.

Usage:
    python3 sim7600_simulator.py [--gps-lat 47.4991] [--gps-lon 19.0408] \
                                  [--no-fix] [--no-lte]

The simulator prints the virtual port name (e.g. /dev/pts/3) which
you can use as the modem UART port in your firmware config or test harness.

Requirements:
    pip install pyserial

On Linux/macOS: uses pty (pseudo-terminal) pairs.
On Windows:     requires com0com virtual COM port driver.
"""

import argparse
import os
import pty
import select
import sys
import threading
import time
import datetime
import random

# ── Configuration ─────────────────────────────────────────────────────────────

DEFAULT_LAT  = 47.499144   # Budapest
DEFAULT_LON  = 19.040840
DEFAULT_ALT  = 150.7
DEFAULT_SPD  = 0.0
DEFAULT_CRS  = 0.0

# ── Helpers ───────────────────────────────────────────────────────────────────

def decimal_to_nmea(deg: float) -> tuple[float, str]:
    """Convert decimal degrees to NMEA DDDMM.MMMMMM format."""
    sign     = 'N' if deg >= 0 else 'S'
    deg      = abs(deg)
    d        = int(deg)
    m        = (deg - d) * 60.0
    return d * 100.0 + m, sign

def decimal_to_nmea_lon(deg: float) -> tuple[float, str]:
    sign = 'E' if deg >= 0 else 'W'
    deg  = abs(deg)
    d    = int(deg)
    m    = (deg - d) * 60.0
    return d * 100.0 + m, sign

# ── AT command response table ─────────────────────────────────────────────────

class Sim7600Simulator:
    def __init__(self, args):
        self.args      = args
        self.gnss_on   = False
        self.lte_on    = not args.no_lte
        self.pdp_active = False
        self.echo      = True
        self.lat       = args.gps_lat
        self.lon       = args.gps_lon
        self.alt       = DEFAULT_ALT
        self.speed     = DEFAULT_SPD
        self.course    = DEFAULT_CRS
        self._lock     = threading.Lock()

    def _cgpsinfo_str(self) -> str:
        if self.args.no_fix or not self.gnss_on:
            return "+CGPSINFO: ,,,,,,,,\r\n\r\nOK"
        now   = datetime.datetime.utcnow()
        date  = now.strftime("%d%m%y")
        utct  = now.strftime("%H%M%S.0")
        lat_n, ns = decimal_to_nmea(self.lat)
        lon_n, ew = decimal_to_nmea_lon(self.lon)
        return (
            f"+CGPSINFO: {lat_n:.6f},{ns},{lon_n:.6f},{ew},"
            f"{date},{utct},{self.alt:.1f},{self.speed:.1f},{self.course:.1f}"
            "\r\n\r\nOK"
        )

    def handle(self, cmd: str) -> str:
        cmd = cmd.strip()

        # Basic
        if cmd in ("AT", "at"):
            return "OK"
        if cmd.upper() == "ATE0":
            self.echo = False
            return "OK"
        if cmd.upper() == "ATE1":
            self.echo = True
            return "OK"
        if cmd.upper() in ("AT+CMEE=2", "AT+CREG=0"):
            return "OK"

        # Network registration
        if cmd.upper() == "AT+CREG?":
            if self.lte_on:
                return "+CREG: 0,1\r\n\r\nOK"
            return "+CREG: 0,2\r\n\r\nOK"   # searching

        # Signal quality
        if cmd.upper() == "AT+CSQ":
            rssi = random.randint(10, 25)    # -93 to -63 dBm, decent signal
            return f"+CSQ: {rssi},0\r\n\r\nOK"

        # Operator
        if cmd.upper() == "AT+COPS?":
            if self.lte_on:
                return '+COPS: 0,0,"One Hungary",7\r\n\r\nOK'
            return '+COPS: 0,0,"",2\r\n\r\nOK'

        # PDP context
        if cmd.upper().startswith("AT+CGDCONT="):
            return "OK"
        if cmd.upper() == "AT+CGACT=1,1":
            if self.lte_on:
                self.pdp_active = True
                return "OK"
            return "+CME ERROR: no service"
        if cmd.upper() == "AT+CGACT=0,1":
            self.pdp_active = False
            return "OK"
        if cmd.upper() == "AT+CGPADDR=1":
            if self.pdp_active:
                return "+CGPADDR: 1,10.20.30.40\r\n\r\nOK"
            return "+CME ERROR: no PDP context"

        # GNSS
        if cmd.upper() in ("AT+CGPS=1", "AT+CGPS=1,1"):
            self.gnss_on = True
            return "OK"
        if cmd.upper() == "AT+CGPS=0":
            self.gnss_on = False
            return "OK"
        if cmd.upper() == "AT+CGPS?":
            return f"+CGPS: {1 if self.gnss_on else 0},1\r\n\r\nOK"
        if cmd.upper() == "AT+CGPSINFO":
            return self._cgpsinfo_str()

        # Fallback
        print(f"  [SIM] Unknown command: {repr(cmd)}", file=sys.stderr)
        return "ERROR"

# ── Virtual serial port (Linux/macOS pty) ─────────────────────────────────────

def run_pty_simulator(sim: Sim7600Simulator):
    master_fd, slave_fd = pty.openpty()
    slave_name = os.ttyname(slave_fd)

    print(f"\n{'='*50}")
    print(f"  SIM7600 Simulator running")
    print(f"  Virtual port: {slave_name}")
    print(f"  Options: LTE={'ON' if not sim.args.no_lte else 'OFF'}, "
          f"GPS fix={'OFF' if sim.args.no_fix else 'ON'}")
    print(f"  GPS position: {sim.lat:.6f}, {sim.lon:.6f}")
    print(f"  Press Ctrl+C to stop")
    print(f"{'='*50}\n")

    buf = b""
    try:
        while True:
            r, _, _ = select.select([master_fd], [], [], 0.05)
            if r:
                data = os.read(master_fd, 256)
                buf += data
                while b"\n" in buf or b"\r" in buf:
                    # Find line end
                    for sep in (b"\r\n", b"\r", b"\n"):
                        idx = buf.find(sep)
                        if idx >= 0:
                            line = buf[:idx].decode("ascii", errors="replace").strip()
                            buf  = buf[idx + len(sep):]
                            if not line:
                                continue
                            print(f"  [FW→SIM] {repr(line)}")
                            response = sim.handle(line)
                            reply = (response + "\r\n").encode()
                            print(f"  [SIM→FW] {repr(response)}")
                            os.write(master_fd, reply)
                            break
                    else:
                        break
    except KeyboardInterrupt:
        print("\nSimulator stopped.")
    finally:
        os.close(master_fd)
        os.close(slave_fd)

# ── Interactive mode (stdin/stdout) for quick testing ─────────────────────────

def run_interactive(sim: Sim7600Simulator):
    print("\nSIM7600 Simulator — interactive mode")
    print("Type AT commands and press Enter. Ctrl+C to quit.\n")
    try:
        while True:
            line = input("AT> ").strip()
            if not line:
                continue
            response = sim.handle(line)
            print(response)
    except (KeyboardInterrupt, EOFError):
        print("\nDone.")

# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="SIM7600 AT command simulator")
    parser.add_argument("--gps-lat",    type=float, default=DEFAULT_LAT,
                        help="GPS latitude (decimal degrees, default: Budapest)")
    parser.add_argument("--gps-lon",    type=float, default=DEFAULT_LON,
                        help="GPS longitude (decimal degrees)")
    parser.add_argument("--no-fix",     action="store_true",
                        help="Simulate no GNSS fix (returns empty CGPSINFO)")
    parser.add_argument("--no-lte",     action="store_true",
                        help="Simulate no LTE network registration")
    parser.add_argument("--interactive", "-i", action="store_true",
                        help="Interactive mode (stdin/stdout, no virtual port)")
    args = parser.parse_args()

    sim = Sim7600Simulator(args)

    if args.interactive or not hasattr(pty, "openpty"):
        run_interactive(sim)
    else:
        run_pty_simulator(sim)

if __name__ == "__main__":
    main()
