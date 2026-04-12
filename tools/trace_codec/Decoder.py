from __future__ import annotations

import argparse
from pathlib import Path

from codec_common import (
    EV_KEY_DOWN,
    EV_KEY_UP,
    EV_MOUSE_MOVE,
    EV_MOUSE_WHEEL,
    KEYBOARD_TEXT,
    MOUSE_TEXT,
    TRACE_SOURCE_C,
    TraceEvent,
    format_time_ns,
    key_label_from_event,
    load_trace_events,
    mouse_button_spec,
)


def default_input_path() -> Path:
    return TRACE_SOURCE_C


def keyboard_lines(events: list[TraceEvent]) -> list[str]:
    pending: dict[tuple[int, int], TraceEvent] = {}
    lines = [
        "# Keyboard trace",
        "# Format: Key <name>: <press time> (<hold duration>)",
        "# Optional: Key <name>: <press time> + <delay> -m|-k (<hold duration> + <extra hold>)",
        "# Time examples accepted by Encoder.py: 9.25, 9.25 s, 1 m 10.25 s, 1m10.25s",
        "",
    ]

    for event in events:
        if event.event_type == EV_KEY_DOWN:
            pending.setdefault((event.code, event.flags & 0x01), event)
            continue
        if event.event_type != EV_KEY_UP:
            continue

        key_id = (event.code, event.flags & 0x01)
        start_event = pending.pop(key_id, None)
        if start_event is None:
            continue

        hold_ns = max(0, event.t_ns - start_event.t_ns)
        key_name = key_label_from_event(start_event.code, start_event.flags)
        lines.append(f"Key {key_name}: {format_time_ns(start_event.t_ns)} ({format_time_ns(hold_ns)})")

    for start_event in pending.values():
        key_name = key_label_from_event(start_event.code, start_event.flags)
        lines.append(f"Key {key_name}: {format_time_ns(start_event.t_ns)} ({format_time_ns(0)})")

    return lines


def mouse_lines(events: list[TraceEvent]) -> list[str]:
    pending: dict[int, TraceEvent] = {}
    button_up_to_down: dict[int, tuple[int, str]] = {}
    for label in ["Left Click", "Right Click", "Middle Click", "X1 Click", "X2 Click"]:
        down_type, up_type, canonical = mouse_button_spec(label)
        button_up_to_down[up_type] = (down_type, canonical)

    lines = [
        "# Mouse trace",
        "# Move: <time> -> (<x>, <y>)",
        "# Left Click: <time> at (<x>, <y>) (<hold duration>)",
        "# Wheel: <time> at (<x>, <y>) delta <amount>",
        "# Optional delay modifiers also work here, for example: Move: 1.2 s + 0.5 s -k -> (100, 200)",
        "",
    ]

    for event in events:
        if event.event_type == EV_MOUSE_MOVE:
            lines.append(f"Move: {format_time_ns(event.t_ns)} -> ({event.x}, {event.y})")
            continue

        if event.event_type == EV_MOUSE_WHEEL:
            lines.append(f"Wheel: {format_time_ns(event.t_ns)} at ({event.x}, {event.y}) delta {event.wheel}")
            continue

        matched_down = None
        for label in ["Left Click", "Right Click", "Middle Click", "X1 Click", "X2 Click"]:
            down_type, _, _ = mouse_button_spec(label)
            if event.event_type == down_type:
                matched_down = down_type
                break

        if matched_down is not None:
            pending.setdefault(matched_down, event)
            continue

        if event.event_type not in button_up_to_down:
            continue

        down_type, canonical = button_up_to_down[event.event_type]
        start_event = pending.pop(down_type, None)
        if start_event is None:
            continue

        hold_ns = max(0, event.t_ns - start_event.t_ns)
        lines.append(
            f"{canonical}: {format_time_ns(start_event.t_ns)} at ({start_event.x}, {start_event.y}) "
            f"({format_time_ns(hold_ns)})"
        )

    for label in ["Left Click", "Right Click", "Middle Click", "X1 Click", "X2 Click"]:
        down_type, _, canonical = mouse_button_spec(label)
        start_event = pending.get(down_type)
        if start_event is None:
            continue
        lines.append(
            f"{canonical}: {format_time_ns(start_event.t_ns)} at ({start_event.x}, {start_event.y}) "
            f"({format_time_ns(0)})"
        )

    return lines


def write_lines(path: Path, lines: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Decode trace data into readable keyboard and mouse files.")
    parser.add_argument("--input", type=Path, default=default_input_path(), help="Source generated .c file")
    parser.add_argument("--keyboard-out", type=Path, default=KEYBOARD_TEXT, help="Keyboard readable output path")
    parser.add_argument("--mouse-out", type=Path, default=MOUSE_TEXT, help="Mouse readable output path")
    args = parser.parse_args()

    events = load_trace_events(args.input)
    events.sort(key=lambda event: event.t_ns)

    write_lines(args.keyboard_out, keyboard_lines(events))
    write_lines(args.mouse_out, mouse_lines(events))

    print(f"Decoded {len(events)} events from {args.input}")
    print(f"Keyboard file: {args.keyboard_out}")
    print(f"Mouse file: {args.mouse_out}")


if __name__ == "__main__":
    main()
