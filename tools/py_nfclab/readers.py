"""
TRZ File Reader

Simple and efficient reader for TRZ trace files.
Complete implementation matching TRZ format specification.
"""

import json
import tarfile
from pathlib import Path
from typing import Iterator, List, Union

from .models import NFCFrame


class TRZReader:
    """
    Reader for TRZ (trace archive) files.

    TRZ files are tar.gz archives containing frame.json with complete frame metadata.
    """

    def __init__(self, file_path: Union[str, Path]):
        """
        Initialize TRZ reader.

        Args:
            file_path: Path to .trz file
        """
        self.file_path = Path(file_path)
        if not self.file_path.exists():
            raise FileNotFoundError(f"TRZ file not found: {file_path}")
        if not self.file_path.suffix == ".trz":
            raise ValueError(f"File must have .trz extension: {file_path}")

    def read_frames(self) -> Iterator[NFCFrame]:
        """
        Read all frames from TRZ file.

        Yields:
            NFCFrame objects with complete TRZ metadata

        Raises:
            ValueError: If TRZ format is invalid
        """
        with tarfile.open(self.file_path, "r:gz") as tar:
            # Extract frame.json
            try:
                member = tar.getmember("frame.json")
            except KeyError:
                raise ValueError("TRZ file does not contain frame.json")

            json_file = tar.extractfile(member)
            if not json_file:
                raise ValueError("Failed to extract frame.json")

            # Parse JSON
            data = json.load(json_file)

            if not isinstance(data, dict) or "frames" not in data:
                raise ValueError("Invalid frame.json format: missing 'frames' array")

            # Yield all frames
            for frame_data in data["frames"]:
                try:
                    yield NFCFrame.from_trz_dict(frame_data)
                except (KeyError, ValueError) as e:
                    raise ValueError(f"Invalid frame format: {e}")

    def read_all(self) -> List[NFCFrame]:
        """
        Read all frames into a list.

        Returns:
            List of NFCFrame objects
        """
        return list(self.read_frames())

    def __iter__(self) -> Iterator[NFCFrame]:
        """Allow iteration over reader"""
        return self.read_frames()


def read_trz(file_path: Union[str, Path]) -> List[NFCFrame]:
    """
    Convenience function to read all frames from TRZ file.

    Args:
        file_path: Path to .trz file

    Returns:
        List of NFCFrame objects

    Example:
        frames = read_trz("trace.trz")
        for frame in frames:
            print(frame)
    """
    reader = TRZReader(file_path)
    return reader.read_all()
