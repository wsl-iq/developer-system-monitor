"""
Logging configuration for Developer System Monitor.
"""

import logging
import os
from pathlib import Path
from logging.handlers import RotatingFileHandler
from typing import Optional

def setup_logging(log_dir: Optional[str] = None, level: int = logging.INFO) -> logging.Logger:
    if log_dir is None:
        log_dir = Path(__file__).parent.parent / 'logs'
    
    # Create log directory if it doesn't exist
    os.makedirs(log_dir, exist_ok=True)
    
    # Configure root logger
    root_logger = logging.getLogger()
    root_logger.setLevel(level)
    
    # Remove existing handlers
    for handler in root_logger.handlers[:]:
        root_logger.removeHandler(handler)
    
    # Create formatters
    detailed_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    simple_formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s - %(message)s'
    )
    
    # Console handler
    console_handler = logging.StreamHandler()
    console_handler.setLevel(level)
    console_handler.setFormatter(simple_formatter)
    root_logger.addHandler(console_handler)
    
    # Application log file
    app_log_file = os.path.join(log_dir, 'application.log')
    app_handler = RotatingFileHandler(
        app_log_file, maxBytes=10*1024*1024, backupCount=5
    )
    app_handler.setLevel(level)
    app_handler.setFormatter(detailed_formatter)
    root_logger.addHandler(app_handler)
    
    # Error log file
    error_log_file = os.path.join(log_dir, 'error.log')
    error_handler = RotatingFileHandler(
        error_log_file, maxBytes=10*1024*1024, backupCount=5
    )
    error_handler.setLevel(logging.ERROR)
    error_handler.setFormatter(detailed_formatter)
    root_logger.addHandler(error_handler)
    
    # Performance log file
    perf_log_file = os.path.join(log_dir, 'performance.log')
    perf_handler = RotatingFileHandler(
        perf_log_file, maxBytes=10*1024*1024, backupCount=5
    )
    perf_handler.setLevel(logging.INFO)
    perf_handler.setFormatter(detailed_formatter)
    
    # Create a filter for performance logs
    class PerformanceFilter(logging.Filter):
        def filter(self, record):
            return record.name.startswith('performance')
    
    perf_handler.addFilter(PerformanceFilter())
    root_logger.addHandler(perf_handler)
    
    # Log initialization
    root_logger.info("Logging initialized")
    root_logger.info(f"Log directory: {log_dir}")
    return root_logger


def get_logger(name: str) -> logging.Logger:
    return logging.getLogger(name)