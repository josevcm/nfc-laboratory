"""
Frame Data Writers

Export NFC frames to various formats.
"""

import json
from pathlib import Path
from typing import Iterable, Optional, Union

from .models import NFCFrame


def write_json(
    frames: Iterable[NFCFrame],
    file_path: Union[str, Path],
    indent: Optional[int] = 2,
) -> None:
    """
    Write frames to JSON file in TRZ frame.json format.

    Args:
        frames: Iterable of NFCFrame objects
        file_path: Output file path
        indent: JSON indentation (default: 2, None for compact)

    Example:
        frames = read_trz("input.trz")
        write_json(frames, "output.json")
    """
    file_path = Path(file_path)

    # Convert all frames to dictionaries
    frame_dicts = [frame.to_dict() for frame in frames]

    # Create TRZ-compatible JSON structure
    data = {"frames": frame_dicts}

    # Write to file
    with open(file_path, "w") as f:
        json.dump(data, f, indent=indent)


def write_jsonl(
    frames: Iterable[NFCFrame],
    file_path: Union[str, Path],
) -> None:
    """
    Write frames to JSON Lines format (one frame per line).

    Args:
        frames: Iterable of NFCFrame objects
        file_path: Output file path

    Example:
        frames = read_trz("input.trz")
        write_jsonl(frames, "output.jsonl")
    """
    file_path = Path(file_path)

    with open(file_path, "w") as f:
        for frame in frames:
            json.dump(frame.to_dict(), f)
            f.write("\n")


def frames_to_json(frames: Iterable[NFCFrame], indent: Optional[int] = 2) -> str:
    """
    Convert frames to JSON string in TRZ format.

    Args:
        frames: Iterable of NFCFrame objects
        indent: JSON indentation (default: 2, None for compact)

    Returns:
        JSON string

    Example:
        frames = read_trz("input.trz")
        json_str = frames_to_json(frames)
        print(json_str)
    """
    frame_dicts = [frame.to_dict() for frame in frames]
    data = {"frames": frame_dicts}
    return json.dumps(data, indent=indent)
