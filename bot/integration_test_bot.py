#!/usr/bin/env python3
"""Simple end-to-end MUD integration bot.

This script starts a BlinkenMUD server process, connects via TCP, creates a
throwaway character, executes in-game actions, validates responses, and then
exits cleanly.
"""

from __future__ import annotations

import argparse
import json
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
EXIT_RE = re.compile(r"\[\s*exits\s*:\s*([^\]]+)\]", re.IGNORECASE)
ROOM_MOB_RE = re.compile(r"^([A-Za-z][A-Za-z'\-\s]{1,30})\s+is\s+here", re.IGNORECASE)
SCORE_HP_RE = re.compile(r"\b(\d+)\/(\d+)\s+hp\b", re.IGNORECASE)
MOVE_COMMANDS = ["north", "east", "south", "west", "up", "down"]
REVERSE_DIRECTION = {
    "north": "south",
    "south": "north",
    "east": "west",
    "west": "east",
    "up": "down",
    "down": "up",
}


class IntegrationAssertionError(RuntimeError):
    pass


def log_event(event: str, **context: object) -> None:
    payload = {"event": event, **context}
    print(f"[integration-bot] {json.dumps(payload, sort_keys=True)}", flush=True)


def log_action(action: str) -> None:
    print(f"[integration-bot] {action}", flush=True)


def assert_any_contains(text: str, expected_tokens: list[str], *, context: str, details: dict[str, object]) -> None:
    lowered = text.lower()
    if any(token.lower() in lowered for token in expected_tokens):
        return
    raise IntegrationAssertionError(
        "Assertion failure: expected at least one token in response. "
        f"context={context} expected={expected_tokens} details={details} "
        f"encountered_tail={text[-800:]}"
    )


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

    def send_and_capture_prompt(self, command: str, timeout_s: float, allow_reject: bool = False) -> str:
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
        if not allow_reject and "huh?" in response.lower():
            raise RuntimeError(f"Command '{command}' appears to have been rejected.\nResponse:\n{response}")
        return response

    def latest_prompt_stats(self) -> tuple[int, int, int] | None:
        prompts = PROMPT_RE.findall(self.buffer)
        if not prompts:
            return None
        latest = prompts[-1]
        match = re.search(r"(\d+)hp\s+(\d+)m\s+(\d+)mv", latest, re.IGNORECASE)
        if not match:
            return None
        return int(match.group(1)), int(match.group(2)), int(match.group(3))


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


def run_bot(host: str, port: int, timeout_s: float, name: str, password: str, sweep_duration_s: float) -> None:
    session = MudSession(host=host, port=port, timeout_s=timeout_s)
    try:
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

        # Enter the game and validate a mini player-like session.
        time.sleep(1)
        finish_login_banner(session, timeout_s)

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

        # Aggressive progression and regression sweep across common game systems.
        aggressive_progression_sweep(session=session, timeout_s=timeout_s, duration_s=sweep_duration_s)

        # Validate CGI script output while server is still running.
        validate_cgi_output(timeout_s)

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


def extract_exits(look_text: str) -> list[str]:
    match = EXIT_RE.search(look_text)
    if not match:
        return []
    exits = []
    for token in re.split(r"\s+", match.group(1).strip().lower()):
        token = token.strip(",. ")
        if token in MOVE_COMMANDS:
            exits.append(token)
    return exits


def extract_room_mob_keywords(look_text: str) -> list[str]:
    keywords = []
    for raw_line in look_text.splitlines():
        line = raw_line.strip()
        match = ROOM_MOB_RE.match(line)
        if not match:
            continue
        words = [w.lower() for w in re.findall(r"[A-Za-z][A-Za-z'\-]*", match.group(1))]
        words = [w for w in words if w not in {"a", "an", "the", "some", "pair", "of"}]
        if words:
            keywords.append(words[-1])
    return list(dict.fromkeys(keywords))


def parse_max_hp(score_text: str) -> int | None:
    match = SCORE_HP_RE.search(score_text)
    if not match:
        return None
    return int(match.group(2))


def identify_room(look_text: str) -> str:
    sanitized = re.sub(r"<\s*\d+hp\s+\d+m\s+\d+mv\s*>", "", look_text, flags=re.IGNORECASE)
    sanitized = sanitized.replace("@", ".")
    compact = " ".join(sanitized.split())
    return compact[:220].lower()


def aggressive_progression_sweep(session: MudSession, timeout_s: float, duration_s: float) -> None:
    deadline = time.monotonic() + duration_s
    movement_history: list[str] = []
    seen_rooms: set[str] = set()
    kills = 0
    attempted_attacks = 0
    mobs_seen = 0
    room_graph: dict[str, dict[str, str]] = {}
    explored_edges: set[tuple[str, str]] = set()

    # Build a health baseline from score.
    score_text = session.send_and_capture_prompt("score", timeout_s)
    max_hp = parse_max_hp(score_text) or 30
    log_event("sweep_start", duration_s=duration_s, max_hp=max_hp)

    scripted_moves = [
        "north", "north", "east", "west", "north", "south", "east", "west", "south", "south",
    ]
    scripted_index = 0

    utility_commands = ["score", "inventory", "equipment", "who", "help score", "save"]
    utility_idx = 0
    sweep_steps = 0

    while time.monotonic() < deadline:
        sweep_steps += 1
        look_text = session.send_and_capture_prompt("look", timeout_s)
        if len(look_text.strip()) < 40:
            raise IntegrationAssertionError(
                "Assertion failure: room scan output unexpectedly short. "
                f"context=room_scan details={{'movement_history_tail': {movement_history[-5:]}}} encountered_tail={look_text[-800:]}"
            )
        room_id = identify_room(look_text)
        seen_rooms.add(room_id)

        # Exercise utility commands less frequently to prioritize exploration speed.
        if sweep_steps % 2 == 0:
            command = utility_commands[utility_idx % len(utility_commands)]
            utility_idx += 1
            response = session.send_and_capture_prompt(command, timeout_s, allow_reject=True)
            if len(response.strip()) < 20:
                raise IntegrationAssertionError(
                    "Assertion failure: utility command returned too little output. "
                    f"context=utility_command details={{'command': '{command}', 'room': '{room_id}'}} "
                    f"encountered_tail={response[-800:]}"
                )
            if "huh?" in response.lower():
                raise IntegrationAssertionError(
                    "Assertion failure: utility command rejected by mud. "
                    f"context=utility_command details={{'command': '{command}', 'room': '{room_id}'}} "
                    f"encountered_tail={response[-800:]}"
                )

        if sweep_steps % 5 == 0:
            log_action("checked status")

        # Natural combat behavior: only pick obvious room targets.
        room_targets = extract_room_mob_keywords(look_text)
        if room_targets:
            mobs_seen += len(room_targets)
        current_stats = session.latest_prompt_stats()
        hp_now = current_stats[0] if current_stats else max_hp

        if room_targets and hp_now >= max(12, int(max_hp * 0.60)):
            target = room_targets[0]
            attempted_attacks += 1
            log_event("combat_attempt", target=target, hp_now=hp_now, room=look_text.splitlines()[:2])
            attack_text = session.send_and_capture_prompt(f"consider {target}", timeout_s, allow_reject=True)
            assert_any_contains(
                attack_text,
                ["you would", "looks", "you have no idea", "isn't here", "death will thank"],
                context="combat_consider",
                details={"target": target, "hp_now": hp_now},
            )
            if "you have no idea" not in attack_text.lower() and "isn't here" not in attack_text.lower():
                log_action(f"fighting {target}")
                engage = session.send_and_capture_prompt(f"kill {target}", timeout_s, allow_reject=True)
                assert_any_contains(
                    engage,
                    ["you attack", "you engage", "you hit", "isn't here"],
                    context="combat_engage",
                    details={"target": target, "hp_now": hp_now},
                )
                fight_deadline = time.monotonic() + min(25.0, max(8.0, timeout_s))
                while time.monotonic() < fight_deadline:
                    pulse = session.send_and_capture_prompt("", timeout_s, allow_reject=True)
                    pulse_lower = pulse.lower()
                    stats = session.latest_prompt_stats()
                    if stats:
                        hp_now = stats[0]

                    if "you are dead" in pulse_lower:
                        raise IntegrationAssertionError(
                            f"Assertion failure: character died. context=combat_loop target={target} encountered_tail={pulse[-800:]}"
                        )

                    if hp_now <= max(8, int(max_hp * 0.30)):
                        log_event("combat_retreat", target=target, hp_now=hp_now)
                        log_action(f"retreating from {target}")
                        flee_response = session.send_and_capture_prompt("flee", timeout_s, allow_reject=True)
                        assert_any_contains(
                            flee_response,
                            ["you flee", "panic", "you couldn't escape"],
                            context="combat_flee",
                            details={"target": target, "hp_now": hp_now},
                        )
                        if movement_history:
                            back = REVERSE_DIRECTION.get(movement_history[-1])
                            if back:
                                session.send_and_capture_prompt(back, timeout_s, allow_reject=True)
                        break

                    if any(token in pulse_lower for token in ["you receive", "you have slain", "is dead"]):
                        kills += 1
                        log_event("combat_kill", target=target, kills=kills)
                        log_action(f"looting {target}")
                        session.send_and_capture_prompt("get all corpse", timeout_s, allow_reject=True)
                        inv = session.send_and_capture_prompt("inventory", timeout_s)
                        assert_any_contains(
                            inv,
                            ["you are carrying", "items"],
                            context="post_combat_inventory",
                            details={"target": target, "kills": kills},
                        )
                        break

                    if "you aren't fighting" in pulse_lower or "isn't here" in pulse_lower:
                        break

        # Recovery behavior before continuing progression.
        if hp_now < max(10, int(max_hp * 0.55)):
            log_event("recovery_start", hp_now=hp_now, max_hp=max_hp)
            for _ in range(4):
                rest_response = session.send_and_capture_prompt("rest", timeout_s, allow_reject=True)
                assert_any_contains(
                    rest_response,
                    ["you rest", "you are already resting", "you stop"],
                    context="recovery_rest",
                    details={"hp_now": hp_now},
                )
                session.send_and_capture_prompt("", timeout_s, allow_reject=True)
            stand = session.send_and_capture_prompt("stand", timeout_s, allow_reject=True)
            assert_any_contains(
                stand,
                ["you stand", "you are already standing"],
                context="recovery_stand",
                details={"hp_now": hp_now},
            )

        exits = extract_exits(look_text)
        room_graph.setdefault(room_id, {})
        direction = None

        if scripted_index < len(scripted_moves):
            direction = scripted_moves[scripted_index]
            scripted_index += 1

        if direction is None and exits:
            unexplored = [d for d in exits if (room_id, d) not in explored_edges]
            if unexplored:
                direction = unexplored[0]
            else:
                non_backtrack = [d for d in exits if not movement_history or d != REVERSE_DIRECTION.get(movement_history[-1])]
                direction = random.choice(non_backtrack or exits)

        if direction is None:
            previous = REVERSE_DIRECTION.get(movement_history[-1]) if movement_history else None
            probe_dirs = [d for d in MOVE_COMMANDS if d != previous]
            direction = random.choice(probe_dirs or MOVE_COMMANDS)

        if direction:
            explored_edges.add((room_id, direction))
            move_response = session.send_and_capture_prompt(direction, timeout_s, allow_reject=True)
            if len(move_response.strip()) < 20:
                raise IntegrationAssertionError(
                    "Assertion failure: navigation output unexpectedly short. "
                    f"context=navigation_move details={{'direction': '{direction}', 'available_exits': {exits}, 'room': '{room_id}'}} "
                    f"encountered_tail={move_response[-800:]}"
                )
            if "huh?" in move_response.lower():
                raise IntegrationAssertionError(
                    "Assertion failure: navigation command rejected. "
                    f"context=navigation_move details={{'direction': '{direction}', 'available_exits': {exits}, 'room': '{room_id}'}} "
                    f"encountered_tail={move_response[-800:]}"
                )
            new_room = identify_room(move_response)
            room_graph[room_id][direction] = new_room
            reverse = REVERSE_DIRECTION.get(direction)
            if reverse:
                room_graph.setdefault(new_room, {})[reverse] = room_id
            log_action(f"moved {direction}")
            movement_history.append(direction)
            if len(movement_history) > 20:
                movement_history.pop(0)

    if len(seen_rooms) < 3:
        raise IntegrationAssertionError(
            f"Assertion failure: progression too shallow. context=sweep_summary expected_rooms>=3 encountered={len(seen_rooms)}"
        )
    if mobs_seen > 0 and attempted_attacks == 0:
        raise IntegrationAssertionError(
            f"Assertion failure: mobs were seen but combat was never attempted. context=sweep_summary mobs_seen={mobs_seen}"
        )
    log_event(
        "sweep_complete",
        seen_rooms=len(seen_rooms),
        mobs_seen=mobs_seen,
        attempted_attacks=attempted_attacks,
        kills=kills,
        mapped_rooms=len(room_graph),
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Run BlinkenMUD integration bot test")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=19000)
    parser.add_argument("--binary", default=str(ROOT / "bin" / "BlinkenMUD"))
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--sweep-duration", type=float, default=165.0)
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
        run_bot(
            host=args.host,
            port=args.port,
            timeout_s=args.timeout,
            name=name,
            password=password,
            sweep_duration_s=args.sweep_duration,
        )
        print(f"Integration bot succeeded on port {args.port} using character {name}.")
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
