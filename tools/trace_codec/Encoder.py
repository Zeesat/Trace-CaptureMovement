from __future__ import annotations

import argparse
import re
from pathlib import Path

from codec_common import (
    COORD_RE,
    EV_KEY_DOWN,
    EV_KEY_UP,
    EV_MOUSE_MOVE,
    EV_MOUSE_WHEEL,
    KEYBOARD_TEXT,
    MOUSE_TEXT,
    REPLAY_C,
    TraceEvent,
    generate_replay_source,
    key_spec_from_label,
    mouse_button_spec,
    parse_time_expression,
    strip_comment,
)


def split_label_and_body(line: str) -> tuple[str, str]:
    if ":" not in line:
        raise ValueError("Missing ':' separator")
    label, body = line.split(":", 1)
    return label.strip(), body.strip()


def split_duration(body: str) -> tuple[str, str]:
    match = re.search(r"\(([^()]*)\)\s*$", body)
    if not match:
        return body.strip(), "0"
    return body[: match.start()].strip(), match.group(1).strip()


def parse_keyboard_file(path: Path) -> tuple[list[TraceEvent], list[str]]:
    events: list[TraceEvent] = []
    warnings: list[str] = []

    if not path.exists():
        return events, warnings

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = strip_comment(raw_line).strip()
        if not line:
            continue

        try:
            label, body = split_label_and_body(line)
            key_name = re.sub(r"^\s*key\s+", "", label, flags=re.IGNORECASE).strip()
            if not key_name:
                raise ValueError("Missing key name")

            press_text, hold_text = split_duration(body)
            press_ns = parse_time_expression(press_text)
            hold_ns = parse_time_expression(hold_text)
            code, flags = key_spec_from_label(key_name)

            events.append(TraceEvent(press_ns, EV_KEY_DOWN, flags, code, 0, 0, 0))
            events.append(TraceEvent(press_ns + max(0, hold_ns), EV_KEY_UP, flags, code, 0, 0, 0))
        except Exception as exc:
            warnings.append(f"{path.name}:{line_number}: {exc}")

    return events, warnings


def parse_mouse_file(path: Path) -> tuple[list[TraceEvent], list[str]]:
    events: list[TraceEvent] = []
    warnings: list[str] = []

    if not path.exists():
        return events, warnings

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = strip_comment(raw_line).strip()
        if not line:
            continue

        try:
            label, body = split_label_and_body(line)
            normalized_label = re.sub(r"[^a-z0-9]+", "", label.lower())

            if normalized_label == "move":
                coord_match = COORD_RE.search(body)
                if coord_match is None:
                    raise ValueError("Missing move coordinates")
                time_text = body[: coord_match.start()].replace("->", " ").replace("at", " ").strip()
                time_ns = parse_time_expression(time_text)
                x = int(coord_match.group(1))
                y = int(coord_match.group(2))
                events.append(TraceEvent(time_ns, EV_MOUSE_MOVE, 0, 0, x, y, 0))
                continue

            if normalized_label == "wheel":
                coord_match = COORD_RE.search(body)
                if coord_match is None:
                    raise ValueError("Missing wheel coordinates")
                time_text = body[: coord_match.start()].replace("at", " ").strip()
                time_ns = parse_time_expression(time_text)
                x = int(coord_match.group(1))
                y = int(coord_match.group(2))
                delta_match = re.search(r"(?:delta|wheel)\s*([+-]?\d+)", body[coord_match.end() :], flags=re.IGNORECASE)
                if delta_match is None:
                    tail_numbers = re.findall(r"[+-]?\d+", body[coord_match.end() :])
                    if not tail_numbers:
                        raise ValueError("Missing wheel delta")
                    delta = int(tail_numbers[-1])
                else:
                    delta = int(delta_match.group(1))
                events.append(TraceEvent(time_ns, EV_MOUSE_WHEEL, 0, 0, x, y, delta))
                continue

            down_type, up_type, _ = mouse_button_spec(label)
            primary_body, hold_text = split_duration(body)
            coord_match = COORD_RE.search(primary_body)
            if coord_match is None:
                raise ValueError("Missing button coordinates")

            time_text = primary_body[: coord_match.start()].replace("at", " ").replace("->", " ").strip()
            press_ns = parse_time_expression(time_text)
            hold_ns = parse_time_expression(hold_text)
            x = int(coord_match.group(1))
            y = int(coord_match.group(2))

            events.append(TraceEvent(press_ns, down_type, 0, 0, x, y, 0))
            events.append(TraceEvent(press_ns + max(0, hold_ns), up_type, 0, 0, x, y, 0))
        except Exception as exc:
            warnings.append(f"{path.name}:{line_number}: {exc}")

    return events, warnings


def main() -> None:
    parser = argparse.ArgumentParser(description="Encode readable keyboard and mouse files into replay C.")
    parser.add_argument("--keyboard", type=Path, default=KEYBOARD_TEXT, help="Keyboard readable input")
    parser.add_argument("--mouse", type=Path, default=MOUSE_TEXT, help="Mouse readable input")
    parser.add_argument("--output", type=Path, default=REPLAY_C, help="Generated replay C output")
    args = parser.parse_args()

    keyboard_events, keyboard_warnings = parse_keyboard_file(args.keyboard)
    mouse_events, mouse_warnings = parse_mouse_file(args.mouse)
    events = keyboard_events + mouse_events
    events.sort(key=lambda event: (event.t_ns, event.event_type, event.code, event.x, event.y, event.wheel))

    generate_replay_source(events, args.output)

    print(f"Wrote {len(events)} events to {args.output}")
    if keyboard_warnings or mouse_warnings:
        print("Warnings:")
        for warning in [*keyboard_warnings, *mouse_warnings]:
            print(f"  - {warning}")


if __name__ == "__main__":
    main()
