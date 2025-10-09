"""
Transaction builder for pairing Poll and Listen frames.

This module provides functionality to group related frames into transactions,
enabling higher-level protocol analysis.
"""

from typing import Callable, List, Optional

from .models import NFCFrame, NFCTransaction
from .protocol import ProtocolParser


class TransactionBuilder:
    """
    Build transactions by pairing Poll frames with their Listen responses.

    This class maintains state to recognize when a Poll frame is followed by
    a Listen response, creating NFCTransaction objects that represent the
    complete command-response pair.
    """

    def __init__(self, timeout: float = 0.1):
        """
        Initialize the transaction builder.

        Args:
            timeout: Maximum time (in seconds) between Poll and Listen frames
                    to be considered part of the same transaction
        """
        self.timeout = timeout
        self.pending_poll: Optional[NFCFrame] = None
        self.transactions: List[NFCTransaction] = []

    def add_frame(self, frame: NFCFrame) -> Optional[NFCTransaction]:
        """
        Add a frame and potentially complete a transaction.

        Args:
            frame: The frame to add

        Returns:
            NFCTransaction if a transaction was completed, None otherwise
        """
        if frame.is_poll():
            # If we have a pending poll, finalize it as incomplete transaction
            if self.pending_poll:
                incomplete = NFCTransaction(poll=self.pending_poll)
                self.transactions.append(incomplete)

            # Store this poll as pending
            self.pending_poll = frame
            return None

        elif frame.is_listen():
            # Check if we have a matching poll
            if self.pending_poll:
                # Check if the listen is within timeout
                time_diff = frame.timestamp - self.pending_poll.timestamp
                if time_diff <= self.timeout:
                    # Create complete transaction
                    transaction = NFCTransaction(poll=self.pending_poll, listen=frame)
                    self.transactions.append(transaction)
                    self.pending_poll = None
                    return transaction
                else:
                    # Timeout - finalize incomplete and ignore this listen
                    incomplete = NFCTransaction(poll=self.pending_poll)
                    self.transactions.append(incomplete)
                    self.pending_poll = None
                    return None

        # Not a poll or listen (e.g., carrier events)
        return None

    def finalize(self) -> Optional[NFCTransaction]:
        """
        Finalize any pending poll as an incomplete transaction.

        Returns:
            NFCTransaction if there was a pending poll, None otherwise
        """
        if self.pending_poll:
            incomplete = NFCTransaction(poll=self.pending_poll)
            self.transactions.append(incomplete)
            self.pending_poll = None
            return incomplete
        return None

    def get_transactions(self) -> List[NFCTransaction]:
        """Get all transactions built so far."""
        return self.transactions

    def clear(self):
        """Clear all transactions and pending state."""
        self.pending_poll = None
        self.transactions.clear()


class StreamProcessor:
    """
    Process a stream of frames through multiple stages.

    This class provides a pipeline for processing frames:
    1. Parse protocol events
    2. Build transactions
    3. Apply custom filters/handlers
    """

    def __init__(self, parser=None, transaction_builder=None) -> None:
        """
        Initialize the stream processor.

        Args:
            parser: ProtocolParser instance (creates default if None)
            transaction_builder: TransactionBuilder instance (creates default if None)
        """
        self.parser = parser or ProtocolParser()
        self.transaction_builder = transaction_builder or TransactionBuilder()
        self.frame_handlers: List[Callable[[NFCFrame], None]] = []
        self.transaction_handlers: List[Callable[[NFCTransaction], None]] = []

    def add_frame_handler(self, handler: Callable[[NFCFrame], None]):
        """Add a handler that is called for each frame."""
        self.frame_handlers.append(handler)

    def add_transaction_handler(self, handler: Callable[[NFCTransaction], None]):
        """Add a handler that is called for each completed transaction."""
        self.transaction_handlers.append(handler)

    def process_frame(self, frame: NFCFrame) -> Optional[NFCTransaction]:
        """
        Process a single frame through the pipeline.

        Args:
            frame: The frame to process

        Returns:
            NFCTransaction if a transaction was completed, None otherwise
        """
        # Parse the frame
        self.parser.parse(frame)

        # Call frame handlers
        for frame_handler in self.frame_handlers:
            frame_handler(frame)

        # Try to build a transaction
        transaction = self.transaction_builder.add_frame(frame)

        # Call transaction handlers if we have a complete transaction
        if transaction:
            for tx_handler in self.transaction_handlers:
                tx_handler(transaction)

        return transaction

    def process_json_line(self, json_line: str) -> Optional[NFCTransaction]:
        """
        Process a JSON line from nfc-lab output.

        Args:
            json_line: JSON string from nfc-lab --json-frames

        Returns:
            NFCTransaction if a transaction was completed, None otherwise
        """
        import json

        line = json_line.strip()

        # Skip empty lines and comments
        if not line or line.startswith("#"):
            return None

        try:
            frame = NFCFrame.from_json(line)
            return self.process_frame(frame)
        except json.JSONDecodeError:
            # Ignore non-JSON lines (e.g., nfc-lab status messages)
            return None

    def finalize(self) -> Optional[NFCTransaction]:
        """Finalize any pending transactions."""
        return self.transaction_builder.finalize()

    def get_transactions(self) -> List[NFCTransaction]:
        """Get all transactions processed so far."""
        return self.transaction_builder.get_transactions()
