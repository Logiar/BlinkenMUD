#!/usr/bin/env python3
"""Simple end-to-end MUD integration bot.

This script starts a BlinkenMUD server process, connects via TCP, creates a
throwaway character, executes in-game actions, validates responses, and then
exits cleanly.
"""

from __future__ import annotations

import argparse
import random
import re
import string
import socket
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
AREA_DIR = ROOT / "area"
PLAYER_DIR = ROOT / "player"
WIZLIST_CGI = ROOT / "src" / "wizlist.cgi"
PROMPT_RE = re.compile(r"<\s*\d+hp\s+\d+m\s+\d+mv\s*>", re.IGNORECASE)


class MudSession:
    def __init__(self, host: str, port: int, timeout_s: float) -> None:
        self.sock = socket.create_connection((host, port), timeout=timeout_s)
        self.sock.settimeout(timeout_s)
        self.buffer = ""

    def close(self) -> None:
        try:
            self.sock.close()
        except OSError:
            pass

    def send_line(self, line: str) -> None:
        self.sock.sendall(line.encode("utf-8") + b"\n")

    @staticmethod
    def _clean(chunk: bytes) -> str:
        text = chunk.decode("latin1", errors="ignore")
        # Strip basic telnet command sequences (IAC + cmd + opt).
        text = re.sub(r"\xff[\xfb\xfc\xfd\xfe].", "", text)
        # Strip ANSI color codes.
        text = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", text)
        return text

    def _recv_into_buffer(self) -> bool:
        try:
            chunk = self.sock.recv(8192)
        except socket.timeout:
            return False
        if not chunk:
            return False
        self.buffer += self._clean(chunk)
        return True

    def read_until(self, patterns: list[str], timeout_s: float) -> str:
        deadline = time.monotonic() + timeout_s
        lowered = [p.lower() for p in patterns]

        while time.monotonic() < deadline:
            if any(pattern in self.buffer.lower() for pattern in lowered):
                return self.buffer
            self._recv_into_buffer()

        raise RuntimeError(
            f"Timed out waiting for one of: {patterns}.\n"
            f"Last output:\n{self.buffer[-3000:]}"
        )

    def send_and_expect(self, command: str, expected_patterns: list[str], timeout_s: float) -> str:
        start = len(self.buffer)
        self.send_line(command)

        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            response = self.buffer[start:]
            if PROMPT_RE.search(response):
                break
            self._recv_into_buffer()
        else:
            raise RuntimeError(f"Timed out waiting for prompt after command '{command}'.")

        response = self.buffer[start:]
        lowered = response.lower()
        if not any(p.lower() in lowered for p in expected_patterns):
            raise RuntimeError(
                f"Command '{command}' missing expected output {expected_patterns}.\n"
                f"Response:\n{response[-2000:]}"
            )
        if "huh?" in lowered:
            raise RuntimeError(f"Command '{command}' appears to have been rejected.\nResponse:\n{response}")
        return response




def wait_for_server_ready(process: subprocess.Popen[str], timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    output = ""

    while time.monotonic() < deadline:
        line = process.stdout.readline()
        if line:
            output += line
            if "ready to rock" in line.lower():
                return
        elif process.poll() is not None:
            break

    raise RuntimeError(
        "MUD process did not become ready in time.\n"
        f"Recent output:\n{output[-2000:]}"
    )


def finish_login_banner(session: MudSession, timeout_s: float) -> None:
    """Handle post-login MOTD pagination and settle at command prompt."""

    # Some runs show a paged message that needs Enter.
    if "hit return to continue" in session.buffer.lower():
        session.send_line("")

    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        if PROMPT_RE.search(session.buffer):
            return
        if "hit return to continue" in session.buffer.lower():
            session.send_line("")
        session._recv_into_buffer()

    raise RuntimeError(f"Failed to reach in-game prompt after login.\n{session.buffer[-2000:]}")



def validate_cgi_output(timeout_s: float) -> None:
    """Run a CGI script and ensure the returned payload looks like valid HTML."""

    result = subprocess.run(
        ["perl", str(WIZLIST_CGI)],
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=timeout_s,
        check=False,
    )

    output = result.stdout
    lowered = output.lower()
    if result.returncode != 0:
        raise RuntimeError(
            "wizlist.cgi failed to execute.\n"
            f"exit={result.returncode}\nstdout=\n{result.stdout[-2000:]}\nstderr=\n{result.stderr[-2000:]}"
        )

    required_tokens = ["content-type: text/html", "<html>", "</html>", "<body>"]
    if not all(token in lowered for token in required_tokens):
        raise RuntimeError(
            "wizlist.cgi output did not look like valid HTML CGI output.\n"
            f"stdout=\n{result.stdout[-3000:]}\nstderr=\n{result.stderr[-2000:]}"
        )


def create_character_and_enter_game(session: MudSession, timeout_s: float, name: str, password: str) -> None:
    """Create a new character and settle at in-game prompt."""

    session.read_until(["What name do you wish to use?"], timeout_s)
    session.send_line(name)

    session.read_until(["Did I get that right"], timeout_s)
    session.send_line("y")

    session.read_until(["Give me a password"], timeout_s)
    session.send_line(password)

    session.read_until(["Please retype password"], timeout_s)
    session.send_line(password)

    session.read_until(["What is your race"], timeout_s)
    session.send_line("human")

    session.read_until(["What is your sex"], timeout_s)
    session.send_line("m")

    session.read_until(["Select a class"], timeout_s)
    session.send_line("warrior")

    session.read_until(["Which alignment"], timeout_s)
    session.send_line("g")

    session.read_until(["customize this character"], timeout_s)
    session.send_line("n")

    session.read_until(["Your choice?"], timeout_s)
    session.send_line("sword")

    time.sleep(1)
    finish_login_banner(session, timeout_s)


def run_bot(host: str, port: int, timeout_s: float, name: str, password: str) -> None:
    session = MudSession(host=host, port=port, timeout_s=timeout_s)
    try:
        create_character_and_enter_game(session, timeout_s, name, password)

        # Entered the game; validate a mini player-like session.

        # Core info checks.
        session.send_and_expect("score", ["you are", "level"], timeout_s)
        session.send_and_expect("inventory", ["you are carrying", "survival pack", "map"], timeout_s)
        session.send_and_expect("equipment", ["you are using"], timeout_s)

        # Interactions in starting room.
        session.send_and_expect("look", ["entrance to mud school"], timeout_s)
        session.send_and_expect("look map", ["map", "thera", "midgaard"], timeout_s)
        session.send_and_expect("say integration test online", ["you say"], timeout_s)

        # Move around like a player.
        session.send_and_expect("north", ["school"], timeout_s)
        session.send_and_expect("look", ["school", "the"], timeout_s)
        session.send_and_expect("south", ["entrance to mud school"], timeout_s)

        # Basic utility commands players commonly use.
        session.send_and_expect("help score", ["score"], timeout_s)
        session.send_and_expect("commands", ["command", "help"], timeout_s)
        session.send_and_expect("who", ["players", "[", name.lower()], timeout_s)

        # Validate CGI script output while server is still running.
        validate_cgi_output(timeout_s)

    finally:
        session.close()


def run_regression_cases(session: MudSession, timeout_s: float) -> None:
    """Run targeted regression checks for risky parser and session behavior."""

    # Regression: command parser should tolerate extra whitespace.
    session.send_and_expect("   look    map   ", ["map", "thera", "midgaard"], timeout_s)

    # Regression: long player input should not disconnect/crash process.
    session.send_and_expect("say " + ("x" * 500), ["you say"], timeout_s)

    # Regression: movement parser remains stable with noisy whitespace.
    session.send_and_expect("   north   ", ["school"], timeout_s)
    session.send_and_expect("   south   ", ["entrance to mud school"], timeout_s)


def run_regression_bot(host: str, port: int, timeout_s: float, name: str, password: str) -> None:
    session = MudSession(host=host, port=port, timeout_s=timeout_s)
    try:
        create_character_and_enter_game(session, timeout_s, name, password)
        run_regression_cases(session, timeout_s)
    finally:
        session.close()


def delete_player(name: str) -> None:
    candidates = {
        PLAYER_DIR / name,
        PLAYER_DIR / name.lower(),
        PLAYER_DIR / name.capitalize(),
    }
    for path in candidates:
        if path.exists():
            path.unlink()


def main() -> int:
    parser = argparse.ArgumentParser(description="Run BlinkenMUD integration bot test")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=19000)
    parser.add_argument("--binary", default=str(ROOT / "bin" / "BlinkenMUD"))
    parser.add_argument("--timeout", type=float, default=20.0)
    parser.add_argument(
        "--scenario",
        choices=["smoke", "regression"],
        default="smoke",
        help="Choose test scenario to run.",
    )
    args = parser.parse_args()

    binary = Path(args.binary)
    if not binary.exists():
        raise SystemExit(f"MUD binary not found at {binary}. Build first with: make -C src all")

    # Max name length is 12 chars.
    name = "T" + "".join(random.choice(string.ascii_lowercase) for _ in range(7))
    password = "pass123"

    process = subprocess.Popen(
        [str(binary), str(args.port)],
        cwd=AREA_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    try:
        wait_for_server_ready(process, timeout_s=args.timeout)
        if args.scenario == "smoke":
            run_bot(host=args.host, port=args.port, timeout_s=args.timeout, name=name, password=password)
        else:
            run_regression_bot(host=args.host, port=args.port, timeout_s=args.timeout, name=name, password=password)

        print(
            f"Integration bot ({args.scenario}) succeeded on port {args.port} "
            f"using character {name}."
        )
        return 0
    finally:
        delete_player(name)

        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()


if __name__ == "__main__":
    raise SystemExit(main())
