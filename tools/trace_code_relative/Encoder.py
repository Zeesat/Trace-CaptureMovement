from __future__ import annotations

import argparse
from dataclasses import dataclass
import re
from pathlib import Path

from codec_common import (
    COORD_RE,
    EV_KEY_DOWN,
    EV_KEY_UP,
    EV_MOUSE_MOVE,
    EV_MOUSE_WHEEL,
    KEYBOARD_TEXT,
    MOUSE_FLAG_INITIAL_ABS,
    MOUSE_TEXT,
    TRACE_SOURCE_C,
    TraceEvent,
    generate_replay_source,
    key_spec_from_label,
    mouse_button_spec,
    parse_time_expression,
    strip_comment,
)


CATEGORY_KEYBOARD = "keyboard"
CATEGORY_MOUSE = "mouse"
ALL_CATEGORIES = {CATEGORY_KEYBOARD, CATEGORY_MOUSE}
SCOPE_FLAG_RE = re.compile(r"(?<!\S)-([mk])(?!\S)", re.IGNORECASE)


@dataclass(frozen=True)
class EventSpec:
    relative_ns: int
    event_type: int
    flags: int
    code: int
    x: int
    y: int
    wheel: int


@dataclass(frozen=True)
class TimelineEntry:
    category: str
    base_start_ns: int
    local_start_shift_ns: int
    propagation_shift_ns: int
    affected_categories: frozenset[str]
    sort_order: int
    events: tuple[EventSpec, ...]


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


def parse_scope_flags(text: str) -> tuple[str, frozenset[str]]:
    categories = set(ALL_CATEGORIES)

    def replacer(match: re.Match[str]) -> str:
        flag = match.group(1).lower()
        if flag == "m":
            categories.discard(CATEGORY_MOUSE)
        elif flag == "k":
            categories.discard(CATEGORY_KEYBOARD)
        return " "

    cleaned = SCOPE_FLAG_RE.sub(replacer, text)
    cleaned = re.sub(r"\s+", " ", cleaned).strip()
    return cleaned, frozenset(categories)


def parse_timed_value(text: str, *, allow_scope: bool) -> tuple[int, int, frozenset[str]]:
    cleaned = text.strip()
    categories = frozenset(ALL_CATEGORIES)
    if allow_scope:
        cleaned, categories = parse_scope_flags(cleaned)

    parts = [part.strip() for part in cleaned.split("+") if part.strip()]
    if not parts:
        raise ValueError("Missing time value")

    base_ns = parse_time_expression(parts[0])
    extra_ns = sum(parse_time_expression(part) for part in parts[1:])
    return base_ns, extra_ns, categories


def build_entry(
    *,
    category: str,
    base_start_ns: int,
    start_shift_ns: int,
    hold_shift_ns: int,
    affected_categories: frozenset[str],
    sort_order: int,
    events: list[EventSpec],
) -> TimelineEntry:
    category_in_scope = category in affected_categories
    local_start_shift_ns = start_shift_ns if category_in_scope else 0
    local_hold_shift_ns = hold_shift_ns if category_in_scope else 0

    adjusted_events: list[EventSpec] = []
    for event in events:
        relative_ns = event.relative_ns
        if relative_ns > 0:
            relative_ns += local_hold_shift_ns
        adjusted_events.append(
            EventSpec(relative_ns, event.event_type, event.flags, event.code, event.x, event.y, event.wheel)
        )

    return TimelineEntry(
        category=category,
        base_start_ns=base_start_ns,
        local_start_shift_ns=local_start_shift_ns,
        propagation_shift_ns=start_shift_ns + hold_shift_ns,
        affected_categories=affected_categories,
        sort_order=sort_order,
        events=tuple(adjusted_events),
    )


def parse_keyboard_file(path: Path) -> tuple[list[TimelineEntry], list[str]]:
    entries: list[TimelineEntry] = []
    warnings: list[str] = []

    if not path.exists():
        return entries, warnings

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
            press_ns, start_shift_ns, affected_categories = parse_timed_value(press_text, allow_scope=True)
            hold_ns, hold_shift_ns, hold_categories = parse_timed_value(hold_text, allow_scope=True)
            if hold_categories != frozenset(ALL_CATEGORIES):
                affected_categories = hold_categories
            code, flags = key_spec_from_label(key_name)

            entries.append(
                build_entry(
                    category=CATEGORY_KEYBOARD,
                    base_start_ns=press_ns,
                    start_shift_ns=start_shift_ns,
                    hold_shift_ns=hold_shift_ns,
                    affected_categories=affected_categories,
                    sort_order=line_number,
                    events=[
                        EventSpec(0, EV_KEY_DOWN, flags, code, 0, 0, 0),
                        EventSpec(max(0, hold_ns), EV_KEY_UP, flags, code, 0, 0, 0),
                    ],
                )
            )

            if (start_shift_ns or hold_shift_ns) and not affected_categories:
                warnings.append(f"{path.name}:{line_number}: shift ignored because both -m and -k remove all targets")
        except Exception as exc:
            warnings.append(f"{path.name}:{line_number}: {exc}")

    return entries, warnings


def parse_mouse_file(path: Path) -> tuple[list[TimelineEntry], list[str]]:
    entries: list[TimelineEntry] = []
    warnings: list[str] = []

    if not path.exists():
        return entries, warnings

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = strip_comment(raw_line).strip()
        if not line:
            continue

        try:
            label, body = split_label_and_body(line)
            normalized_label = re.sub(r"[^a-z0-9]+", "", label.lower())

            if normalized_label == "init":
                coord_match = COORD_RE.search(body)
                if coord_match is None:
                    raise ValueError("Missing initial coordinates")
                time_text = body[: coord_match.start()]
                time_text = re.sub(r"\b(?:at|init|start|anchor)\b", " ", time_text, flags=re.IGNORECASE)
                time_text = re.sub(r"\s+", " ", time_text).strip()
                time_ns, start_shift_ns, affected_categories = parse_timed_value(time_text, allow_scope=True)
                x = int(coord_match.group(1))
                y = int(coord_match.group(2))
                entries.append(
                    build_entry(
                        category=CATEGORY_MOUSE,
                        base_start_ns=time_ns,
                        start_shift_ns=start_shift_ns,
                        hold_shift_ns=0,
                        affected_categories=affected_categories,
                        sort_order=line_number,
                        events=[EventSpec(0, EV_MOUSE_MOVE, MOUSE_FLAG_INITIAL_ABS, 0, x, y, 0)],
                    )
                )
                if start_shift_ns and not affected_categories:
                    warnings.append(f"{path.name}:{line_number}: shift ignored because both -m and -k remove all targets")
                continue

            if normalized_label == "move":
                coord_match = COORD_RE.search(body)
                if coord_match is None:
                    raise ValueError("Missing move delta")
                time_text = body[: coord_match.start()]
                time_text = re.sub(r"\b(?:delta|move|->)\b", " ", time_text, flags=re.IGNORECASE)
                time_text = re.sub(r"\s+", " ", time_text).strip()
                time_ns, start_shift_ns, affected_categories = parse_timed_value(time_text, allow_scope=True)
                dx = int(coord_match.group(1))
                dy = int(coord_match.group(2))
                entries.append(
                    build_entry(
                        category=CATEGORY_MOUSE,
                        base_start_ns=time_ns,
                        start_shift_ns=start_shift_ns,
                        hold_shift_ns=0,
                        affected_categories=affected_categories,
                        sort_order=line_number,
                        events=[EventSpec(0, EV_MOUSE_MOVE, 0, 0, dx, dy, 0)],
                    )
                )
                if start_shift_ns and not affected_categories:
                    warnings.append(f"{path.name}:{line_number}: shift ignored because both -m and -k remove all targets")
                continue

            if normalized_label == "wheel":
                coord_match = COORD_RE.search(body)
                working_body = body
                if coord_match is not None:
                    working_body = f"{body[:coord_match.start()].strip()} {body[coord_match.end():].strip()}".strip()
                delta_match = re.search(r"([+-]?\d+)\s*$", working_body)
                if delta_match is None:
                    raise ValueError("Missing wheel delta")
                delta = int(delta_match.group(1))
                time_text = working_body[: delta_match.start()]
                time_text = re.sub(r"\b(?:delta|wheel|at)\b", " ", time_text, flags=re.IGNORECASE)
                time_text = re.sub(r"\s+", " ", time_text).strip()
                time_ns, start_shift_ns, affected_categories = parse_timed_value(time_text, allow_scope=True)
                entries.append(
                    build_entry(
                        category=CATEGORY_MOUSE,
                        base_start_ns=time_ns,
                        start_shift_ns=start_shift_ns,
                        hold_shift_ns=0,
                        affected_categories=affected_categories,
                        sort_order=line_number,
                        events=[EventSpec(0, EV_MOUSE_WHEEL, 0, 0, 0, 0, delta)],
                    )
                )
                if start_shift_ns and not affected_categories:
                    warnings.append(f"{path.name}:{line_number}: shift ignored because both -m and -k remove all targets")
                continue

            down_type, up_type, _ = mouse_button_spec(label)
            primary_body, hold_text = split_duration(body)
            coord_match = COORD_RE.search(primary_body)
            if coord_match is not None:
                primary_body = f"{primary_body[:coord_match.start()].strip()} {primary_body[coord_match.end():].strip()}".strip()

            time_text = re.sub(r"\b(?:at|delta)\b", " ", primary_body, flags=re.IGNORECASE)
            time_text = re.sub(r"\s+", " ", time_text).strip()
            press_ns, start_shift_ns, affected_categories = parse_timed_value(time_text, allow_scope=True)
            hold_ns, hold_shift_ns, hold_categories = parse_timed_value(hold_text, allow_scope=True)
            if hold_categories != frozenset(ALL_CATEGORIES):
                affected_categories = hold_categories

            entries.append(
                build_entry(
                    category=CATEGORY_MOUSE,
                    base_start_ns=press_ns,
                    start_shift_ns=start_shift_ns,
                    hold_shift_ns=hold_shift_ns,
                    affected_categories=affected_categories,
                    sort_order=line_number,
                    events=[
                        EventSpec(0, down_type, 0, 0, 0, 0, 0),
                        EventSpec(max(0, hold_ns), up_type, 0, 0, 0, 0, 0),
                    ],
                )
            )

            if (start_shift_ns or hold_shift_ns) and not affected_categories:
                warnings.append(f"{path.name}:{line_number}: shift ignored because both -m and -k remove all targets")
        except Exception as exc:
            warnings.append(f"{path.name}:{line_number}: {exc}")

    return entries, warnings


def build_events(entries: list[TimelineEntry]) -> list[TraceEvent]:
    cumulative_shift = {
        CATEGORY_KEYBOARD: 0,
        CATEGORY_MOUSE: 0,
    }
    events: list[TraceEvent] = []

    for entry in sorted(entries, key=lambda item: (item.base_start_ns, item.sort_order, item.category)):
        effective_start_ns = entry.base_start_ns + cumulative_shift[entry.category] + entry.local_start_shift_ns

        for event in entry.events:
            events.append(
                TraceEvent(
                    effective_start_ns + event.relative_ns,
                    event.event_type,
                    event.flags,
                    event.code,
                    event.x,
                    event.y,
                    event.wheel,
                )
            )

        for category in entry.affected_categories:
            cumulative_shift[category] += entry.propagation_shift_ns

    return events


def main() -> None:
    parser = argparse.ArgumentParser(description="Encode readable keyboard and relative mouse files into replay C.")
    parser.add_argument("--keyboard", type=Path, default=KEYBOARD_TEXT, help="Keyboard readable input")
    parser.add_argument("--mouse", type=Path, default=MOUSE_TEXT, help="Relative mouse readable input")
    parser.add_argument("--output", type=Path, default=TRACE_SOURCE_C, help="Generated replay C output")
    args = parser.parse_args()

    keyboard_entries, keyboard_warnings = parse_keyboard_file(args.keyboard)
    mouse_entries, mouse_warnings = parse_mouse_file(args.mouse)
    events = build_events(keyboard_entries + mouse_entries)
    events.sort(key=lambda event: (event.t_ns, event.event_type, event.code, event.x, event.y, event.wheel))

    generate_replay_source(events, args.output)

    print(f"Wrote {len(events)} events to {args.output}")
    if keyboard_warnings or mouse_warnings:
        print("Warnings:")
        for warning in [*keyboard_warnings, *mouse_warnings]:
            print(f"  - {warning}")


if __name__ == "__main__":
    main()
